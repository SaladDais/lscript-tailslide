#include "linden_common.h"
#include "llerrorcontrol.h"
#include "lluuid.h"

#include <tailslide/tailslide.hh>
#include <tailslide/passes/lso/script_compiler.hh>
#include <tailslide/passes/mono/script_compiler.hh>

#include "../lscript_rt_interface.h"
#include "../lscript_compile/lscript_tree.h"
#include "lscript_resource.h"
#include "fuzz_utils.h"


void indra_restart  (FILE * input_file );
int indra_parse();
void indra_set_in  ( FILE * _in_str  );
void indra_set_out  ( FILE * _out_str  );


BOOL lscript_harness_compile(const char* src_filename, const char* dst_filename,
							 const char* err_filename, BOOL compile_to_mono, const char* class_name, BOOL is_god_like)
{
	BOOL			b_parse_ok = FALSE;
	BOOL			b_dummy = FALSE;
	U64				b_dummy_count = FALSE;
	LSCRIPTType		type = LST_NULL;

	gInternalColumn = 0;
	gInternalLine = 0;
	gScriptp = NULL;

	gErrorToText.init();
	init_temp_jumps();
	gAllocationManager = new LLScriptAllocationManager();

	FILE *our_in = LLFile::fopen(std::string(src_filename), "rb");
	if (our_in)
	{
		indra_set_in(our_in);
		FILE *our_out = LLFile::fopen(std::string(err_filename), "wb");
		indra_set_out(our_out);
		// Reset the lexer's internal buffering.

		indra_restart(our_in);

		b_parse_ok = !indra_parse();

		if (b_parse_ok)
		{
#ifdef EMERGENCY_DEBUG_PRINTOUTS
			char compiled[256];
			sprintf(compiled, "%s.o", src_filename);
			LLFILE* compfile;
			compfile = LLFile::fopen(compiled, "w");
#endif

			if(dst_filename)
			{
				gScriptp->setBytecodeDest(dst_filename);
			}

			gScriptp->mGodLike = is_god_like;

			gScriptp->setClassName(class_name);

			gScopeStringTable = new LLStringTable(16384);
#ifdef EMERGENCY_DEBUG_PRINTOUTS
			gScriptp->recurse(compfile, 0, 4, LSCP_PRETTY_PRINT, LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
#endif
			gScriptp->recurse(our_out, 0, 0, LSCP_PRUNE,		 LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
			gScriptp->recurse(our_out, 0, 0, LSCP_SCOPE_PASS1, LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
			gScriptp->recurse(our_out, 0, 0, LSCP_SCOPE_PASS2, LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
			gScriptp->recurse(our_out, 0, 0, LSCP_TYPE,		 LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
			if (!gErrorToText.getErrors())
			{
				gScriptp->recurse(our_out, 0, 0, LSCP_RESOURCE, LSPRUNE_INVALID,		 b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);

#ifdef EMERGENCY_DEBUG_PRINTOUTS
				gScriptp->recurse(our_out, 0, 0, LSCP_EMIT_ASSEMBLY, LSPRUNE_INVALID,  b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
#endif
				if(TRUE == compile_to_mono)
				{
					gScriptp->recurse(our_out, 0, 0, LSCP_EMIT_CIL_ASSEMBLY, LSPRUNE_INVALID,  b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
				}
				else
				{
					gScriptp->recurse(our_out, 0, 0, LSCP_EMIT_BYTE_CODE, LSPRUNE_INVALID, b_dummy, NULL, type, type, b_dummy_count, NULL, NULL, 0, NULL, 0, NULL);
				}
			}
			delete gScopeStringTable;
			gScopeStringTable = NULL;
#ifdef EMERGENCY_DEBUG_PRINTOUTS
			fclose(compfile);
#endif
		}
		fclose(our_out);
		fclose(our_in);
	}

	delete gAllocationManager;
	delete gScopeStringTable;

	return b_parse_ok && !gErrorToText.getErrors();
}


static bool initialized = false;
static bool compile_cil = false;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (!initialized)
    {
        init_supported_expressions();
        Tailslide::tailslide_init_builtins(nullptr);
        initialized = true;
        compile_cil = getenv("COMPILE_CIL") != nullptr;
    }

    const char* file = buf_to_file(data, size);
    if (!file) {
        exit(EXIT_FAILURE);
    }

	LLUUID script_id = LLUUID::null;
    char *pathname = strdup("/dev/shm/lso-fuzz-XXXXXX");
    int fd = mkstemp(pathname);
    if (fd == -1) {
        free(pathname);
        return 1;
    }
    close(fd);

	bool failed = !lscript_harness_compile(file, pathname, "/dev/null", compile_cil, script_id.asString().c_str(), 0);
    if (delete_file(pathname) != 0) {
        exit(EXIT_FAILURE);
    }

	if (failed) {
        if (delete_file(file) != 0) {
            exit(EXIT_FAILURE);
        }
        return 0;
    }

    std::ifstream in;
    in.open(pathname, std::ifstream::in | std::ifstream::binary);
    std::stringstream sstr;
    sstr << in.rdbuf();
    const std::string expected(sstr.str());
    in.close();

	Tailslide::ScopedScriptParser parser(nullptr);
	auto script = parser.parseLSL(file);

	if (script) {
		script->collectSymbols();
		script->determineTypes();
		script->recalculateReferenceData();
		script->propagateValues();
		script->finalPass();
        script->checkSymbols();
		script->validateGlobals(compile_cil);
		parser.logger.finalize();
		llassert_always_msg(!parser.logger.getErrors(), "unexpected script errors!");

        std::string actual;
        if (compile_cil) {
            Tailslide::MonoScriptCompiler visitor(&parser.allocator);
            script->visit(&visitor);
            actual = visitor.mCIL.str();
        } else {
            Tailslide::LSOScriptCompiler visitor(&parser.allocator);
            script->visit(&visitor);
            actual = {(const char*)visitor.mScriptBS.data(), visitor.mScriptBS.size()};
        }

		assert(expected == actual);
	}

    if (delete_file(file) != 0) {
        exit(EXIT_FAILURE);
    }

	return EXIT_SUCCESS;
}
