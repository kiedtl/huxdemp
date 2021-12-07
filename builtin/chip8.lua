-- CAVEATS:
-- CHIP-8 opcodes are always aligned to 2. If we're on an odd offset,
-- disassembling will be completely screwed.

local huxdemp = require("huxdemp")

local M = {}

local use_color = nil

local styles = {
    chip8 = "",
    schip = "\x1b[38;5;6m",
    xochip = "\x1b[38;5;5m",
    continued = "\x1b[38;5;7m",
    unknown = "\x1b[38;5;8m",
    skipped = "\x1b[38;5;7m",
}

local pbuffer = nil
local skip_next_op = false

local function decode(buffer, index)
    local op = buffer[index] << 8 | buffer[index + 1]

    local P      = (op >> 12);
    local Y      = (op >>  4) & 0xF;
    local N      = (op >>  0) & 0xF;
    local NN     = (op >>  0) & 0xFF;

    local opstring, optype = ({
        [0x0] = function ()
            local _switch = {
                [0xC] = function () return "00CN", "schip" end,
                [0xD] = function () return "00DN", "xochip" end,
                ["d"] = function ()
                    local _switch = {
                        [0x00E0] = function () return "00E0" end,
                        [0x00EE] = function () return "00EE" end,
                        [0x00FB] = function () return "00FB", "schip" end,
                        [0x00FC] = function () return "00FC", "schip" end,
                        [0x00FD] = function () return "00FD", "schip" end,
                        [0x00FE] = function () return "00FE", "schip" end,
                        [0x00FF] = function () return "00FF", "schip" end,
                        ["d"] = function () return "????", "unknown" end,
                    }
                    return (_switch[op] or _switch["d"])()
                end
            }
            return (_switch[Y] or _switch["d"])()
        end,
        [0x1] = function() return "1NNN" end,
        [0x2] = function() return "2NNN" end,
        [0x3] = function() return "3XNN" end,
        [0x4] = function() return "4XNN" end,
        [0x5] = function()
            local _switch = {
                [0x0] = function () return "5XY0" end,
                [0x2] = function () return "5XY2", "xochip" end,
                [0x3] = function () return "5XY3", "xochip" end,
                ["d"] = function () return "????", "unknown" end,
            }
            return (_switch[N] or _switch["d"])()
        end,
        [0x6] = function() return "6XNN" end,
        [0x7] = function() return "7XNN" end,
        [0x8] = function()
            local _switch = {
                [0x0] = function () return "8XY0" end,
                [0x1] = function () return "8XY1" end,
                [0x2] = function () return "8XY2" end,
                [0x3] = function () return "8XY3" end,
                [0x4] = function () return "8XY4" end,
                [0x5] = function () return "8XY5" end,
                [0x6] = function () return "8X06" end,
                [0x7] = function () return "8XY7" end,
                [0xE] = function () return "8X0E" end,
                ["d"] = function () return "????", "unknown" end,
            }
            return (_switch[N] or _switch["d"])()
        end,
        [0x9] = function() return "9XY0" end,
        [0xA] = function() return "ANNN" end,
        [0xB] = function() return "BNNN" end,
        [0xC] = function() return "CXNN" end,
        [0xD] = function()
            if N == 0 then return "DXY0", "schip" end
            return "DXYN"
        end,
        [0xE] = function()
            local _switch = {
                [0x9E] = function () return "EX9E" end,
                [0xA1] = function () return "EXA1" end,
                ["d"] = function () return "????", "unknown" end,
            }
            return (_switch[NN] or _switch["d"])()
        end,
        [0xF] = function()
            local _switch = {
                [0x00] = function ()
                    if op == 0xF000 then return "F000", "xochip" end
                    return "????", "unknown"
                end,
                [0x01] = function () return "FX01", "xochip" end,
                [0x02] = function () return "F002", "xochip" end,
                [0x07] = function () return "FX07" end,
                [0x15] = function () return "FX15" end,
                [0x18] = function () return "FX18" end,
                [0x29] = function () return "FX29" end,
                [0x30] = function () return "FX30", "schip" end,
                [0x1E] = function () return "FX1E" end,
                [0x0A] = function () return "FX0A" end,
                [0x33] = function () return "FX33" end,
                [0x33] = function () return "FX3A", "xochip" end,
                [0x55] = function () return "FX55" end,
                [0x65] = function () return "FX65" end,
                [0x75] = function () return "FX75", "schip" end,
                [0x85] = function () return "FX85", "schip" end,
                ["d"] = function () return "????", "unknown" end,
            }
            return (_switch[NN] or _switch["d"])()
        end,
    })[P]()
    return opstring, optype or "chip8"
end

function M.main(buffer, offset, out)
    use_color = use_color or huxdemp.colors_enabled()

    local function s(style, text)
        if use_color then
            return style .. text .. "\x1b[m"
        end
        return text
    end

    local i = 1

    if pbuffer then
        buffer = {table.unpack(pbuffer), table.unpack(buffer)}
        pbuffer = nil
    end

    while i <= #buffer do
        if skip_next_op then
            skip_next_op = false
            out:write(s(styles.skipped, "**** "))
            i = i + 2
            goto continue
        end

        -- Buffer input if we don't have enough bytes.
        if (#buffer + 1) - i < 2 then
            pbuffer = {table.unpack(buffer, i, #buffer)}
            out:write(s(styles.continued, "---- "))
            break
        end

        local opstring, optype = decode(buffer, i)
        out:write(s(styles[optype], opstring .. " "))

        if opstring == "F000" then
            skip_next_op = true
        end

        i = i + 2
    ::continue::
    end

    local linewidth = huxdemp.linewidth()
    if #buffer < linewidth then
        local pad = math.floor(((linewidth / 2) - math.ceil(#buffer / 2)) * 5)
        out:write((" "):rep(pad))
    end
end

return M
