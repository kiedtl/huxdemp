-- References:
-- https://wiki.xxiivv.com/site/uxntal.html

local huxdemp = require("huxdemp")

local M = {}

local use_color = nil

local styles = {
    normal = "",
    skipped = "\x1b[38;5;7m",
    lit = "\x1b[38;5;7m",
    k = "\x1b[38;5;3m",
    r = "\x1b[1m",
    ["2"] = "\x1b[38;5;6m",
}
-- Damned lua doesn't allow referencing table within table
styles.kr  = styles.k .. styles.r
styles.kr2 = styles.k .. styles.r .. styles["2"]
styles.r2  = styles.r .. styles["2"]

local LIT_STATE_WANT_2 = 2
local LIT_STATE_WANT_1 = 1
local LIT_STATE_UNSET  = 0

local lit_state = LIT_STATE_UNSET
local lit_accm = { val = 0, sz = 0 }

local function decode(buffer, index)
    local op = buffer[index]

    local m_keep  = buffer[index] & 0x80
    local m_ret   = buffer[index] & 0x40
    local m_short = buffer[index] & 0x20
    local opcode  = buffer[index] & 0x1F

    local modestring = ""
    if m_keep  ~= 0 then modestring = modestring .. "k" end
    if m_ret   ~= 0 then modestring = modestring .. "r" end
    if m_short ~= 0 then modestring = modestring .. "2" end

    local opstring = ({
        [0x00] = "LIT",
        [0x01] = "INC", [0x02] = "POP", [0x03] = "DUP", [0x04] = "NIP",
        [0x05] = "SWP", [0x06] = "OVR", [0x07] = "ROT",
        [0x08] = "EQU", [0x09] = "NEQ", [0x0A] = "GTH", [0x0B] = "LTH",
        [0x0C] = "JMP", [0x0D] = "JCN", [0x0E] = "JSR",
        [0x0F] = "STH", [0x10] = "LDZ", [0x11] = "STZ", [0x12] = "LDR",
        [0x13] = "STR", [0x14] = "LDA", [0x15] = "STA",
        [0x16] = "DEI", [0x17] = "DEO",
        [0x18] = "ADD", [0x19] = "SUB", [0x1A] = "MUL", [0x1B] = "DIV",
        [0x1C] = "AND", [0x1D] = "ORA", [0x1E] = "EOR", [0x1F] = "SFT",
    })[opcode]

    return opstring, modestring
end

function M.main(buffer, offset, out)
    use_color = use_color or huxdemp.colors_enabled()

    local i = 1
    local cols = 0

    local function puts(style, fmt, ...)
        local text = string.format("%-6s ", string.format(fmt, ...))
        if use_color then
            text = style .. text .. "\x1b[m"
        end
        out:write(text)
        cols = cols + 1
    end

    while i <= #buffer do
        if lit_state ~= LIT_STATE_UNSET then
            if lit_state == LIT_STATE_WANT_2 then
                lit_state = LIT_STATE_WANT_1

                lit_accm.val = buffer[i]
                lit_accm.sz = lit_accm.sz + 8

                puts(styles.skipped, "---")
            elseif lit_state == LIT_STATE_WANT_1 then
                lit_state = LIT_STATE_UNSET

                lit_accm.val = (lit_accm.val << 8) | buffer[i]
                lit_accm.sz = lit_accm.sz + 8

                if lit_accm.sz == 8 then
                    puts(styles.normal, "#%02X", lit_accm.val)
                elseif lit_accm.sz == 16 then
                    puts(styles.normal, "#%04X", lit_accm.val)
                end
                lit_accm = { val = 0, sz = 0 }
            end
        else
            local opstring, modestring = decode(buffer, i)
            local style = styles[modestring] or styles.normal

            if opstring == "LIT" then
                if modestring:match("2") then
                    lit_state = LIT_STATE_WANT_2
                else
                    lit_state = LIT_STATE_WANT_1
                end
                style = styles.lit
            end

            puts(style, "%s", opstring .. modestring)
        end

        i = i + 1
    end

    local linewidth = huxdemp.linewidth()
    if cols < linewidth then
        local pad = math.floor((linewidth - cols) * 7)
        out:write((" "):rep(pad))
    end
end

return M
