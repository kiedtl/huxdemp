#include <assert.h>
#include <err.h>
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#if LUA_VERSION_NUM < 502
#error "huxdemp requires Lua >= v5.2."
#endif

static lua_State *L = NULL;

static void
luau_sdump(lua_State *pL)
{
	fprintf(stderr, "---------- STACK DUMP ----------\n");
	for (int i = lua_gettop(pL); i; --i) {
		int t = lua_type(pL, i);
		switch (t) {
		break; case LUA_TSTRING:
			fprintf(stderr, "%4d: [string] '%s'\n", i, lua_tostring(pL, i));
		break; case LUA_TBOOLEAN:
			fprintf(stderr, "%4d: [bool]   '%s'\n", i,
				lua_toboolean(pL, i) ? "true" : "false");
		break; case LUA_TNUMBER:
			fprintf(stderr, "%4d: [number] '%g'\n", i, lua_tonumber(pL, i));
		break; case LUA_TNIL:
			fprintf(stderr, "%4d: [nil]\n", i);
		break; default:
			fprintf(stderr, "%4d: [%s] #%d <%p>\n", i, lua_typename(pL, t),
				(int)lua_rawlen(pL, i), lua_topointer(pL, i));
		break;
		}
	}
	fprintf(stderr, "---------- STACK  END ----------\n");
}

static int
luau_panic(lua_State *pL)
{
	char *err = (char *)lua_tostring(pL, -1);
	luaL_traceback(pL, pL, err, 0);
	fprintf(stderr, "Lua error: %s\n\n", lua_tostring(pL, -1));

	errx(1, "Fatal Lua error, exiting.");

	return 0;
}

static void
luau_init(lua_State **pL)
{
        *pL = luaL_newstate();
        assert(*pL);

        luaL_openlibs(*pL);
        luaopen_table(*pL);
        luaopen_string(*pL);
        luaopen_math(*pL);
        luaopen_io(*pL);

        lua_atpanic(*pL, luau_panic);
}

static void
luau_call(lua_State *pL, const char *namespace, const char *fnname, size_t nargs, size_t nret)
{
	/* get function from namespace. */
	if (namespace) {
		lua_getglobal(pL, namespace);
		lua_getfield(pL, -1, fnname);
		lua_remove(pL, -2);
	} else {
		lua_getglobal(pL, fnname);
	}

	/* assert function isn't nil */
	assert(lua_type(L, lua_gettop(L)) != LUA_TNIL);

	/* move function before args. */
	lua_insert(pL, -nargs - 1);

	/* move error func before function and args. */
	size_t errfn_pos = (size_t)lua_gettop(pL) - nargs - 1;
	lua_pushcfunction(pL, luau_panic);
	lua_insert(pL, -nargs - 2);

	if (lua_pcall(pL, nargs, nret, -nargs - 2) == LUA_OK) {
		lua_remove(pL, errfn_pos);
	}
}

static int
fake_pclose(lua_State *pL)
{
	lua_pop(pL, -1);
	return 0;
}

static void
call_plugin(size_t func_index, byte_t *buf, size_t buf_sz, size_t offset, FILE *out)
{
	char *func_name = strdup(options.dfunc_names[func_index]);
	char *plugin_name = func_name;
	char *plugin_func = "main";

	char *dash = strchr(func_name, '-');
	if (dash) {
		*dash = '\0';
		plugin_func = dash + 1;
	}

	lua_newtable(L);
	for (size_t i = 0; i < buf_sz; ++i) {
		lua_pushinteger(L, (lua_Integer)(i + 1));
		lua_pushinteger(L, (lua_Integer)buf[i]);
		lua_settable(L, -3);
	}

	lua_pushinteger(L, (lua_Integer)offset);

	luaL_Stream *p = (luaL_Stream *)lua_newuserdata(L, sizeof(luaL_Stream));
	p->closef = &fake_pclose;
	p->f = out;
	luaL_setmetatable(L, LUA_FILEHANDLE);

	luau_call(L, plugin_name, plugin_func, 3, 0);

	free(func_name);
}

// ---

struct ReaderState {
	char *data;
	size_t size;
};

static const char *
luau_reader(lua_State *pL, void *ud, size_t *size)
{
	(void)pL;
	struct ReaderState *state = (struct ReaderState *)ud;
	if (state->size == 0) return NULL;
	*size = state->size;
	state->size = 0;
	return state->data;
}

static void
luau_evalstring(lua_State *pL, char *name, char *origin, char *stuff)
{
	struct ReaderState state = { stuff, strlen(stuff) };
	int rload = lua_load(pL, luau_reader, (void *)&state, origin, NULL);
	if (rload != LUA_OK) luau_panic(pL);
	int reval = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (reval != LUA_OK) luau_panic(pL);
        lua_setglobal(pL, name);
}

// ---

static int
api_option_linewidth(lua_State *pL)
{
        lua_pushinteger(pL, (lua_Integer)options.linelen);
        return 1;
}

static int
api_color_for(lua_State *pL)
{
	lua_Integer a = luaL_checkinteger(pL, 1);
	if ((byte_t)a != a) {
		lua_pushnil(pL);
	} else {
		lua_pushinteger(pL, styles[(int)a]);
	}
        return 1;
}

static int
api_colors_enabled(lua_State *pL)
{
	lua_pushboolean(pL, options._color);
        return 1;
}

static const struct luaL_Reg huxdemp_lib[] = {
        { "linewidth",      api_option_linewidth },
        { "color_for",      api_color_for },
        { "colors_enabled", api_colors_enabled },
        { NULL, NULL },
};

static int
luau_openlib(lua_State *pL)
{
        char *lib = (char *) luaL_checkstring(pL, 1);

        lua_newtable(L);
        if (!strcmp(lib, "huxdemp")) {
                luaL_setfuncs(pL, huxdemp_lib, 0);
        }

        return 1;
}
