#include "linden_common.h"
#include "llerrorcontrol.h"
#include "lluuid.h"

#include <tailslide/tailslide.hh>
#include <tailslide/passes/lso/script_compiler.hh>
#include <tailslide/passes/mono/script_compiler.hh>

#include "../lscript_rt_interface.h"
#include "../lscript_compile/lscript_tree.h"
#include "lscript_resource.h"

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
    init_supported_expressions();
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


int main(int argc, char **argv) {
    if (argc != 2)
        return 1;

    bool use_tailslide = getenv("USE_TAILSLIDE") != nullptr;
    bool compile_cil = getenv("COMPILE_CIL") != nullptr;

    LLError::initForApplication(".", ".", true /* log to stderr */);
    LLUUID script_id;
    script_id = LLUUID::null;

    Tailslide::tailslide_init_builtins(nullptr);

    if (use_tailslide)
    {
        Tailslide::ScopedScriptParser parser(nullptr);
        auto script = parser.parseLSL(argv[1]);
        if (!script)
            return 1;

        script->collectSymbols();
        script->determineTypes();
        script->recalculateReferenceData();
        script->propagateValues();
        script->finalPass();
        script->checkSymbols();
        script->validateGlobals(compile_cil);
        parser.logger.finalize();
        if (parser.logger.getErrors())
        {
            parser.logger.report();
            return 1;
        }
        if (compile_cil)
        {
            Tailslide::MonoScriptCompiler visitor(&parser.allocator);
            script->visit(&visitor);
            std::ofstream f("/tmp/whatever.cil", std::ios::out | std::ios::binary);
            auto cil = visitor.mCIL.str();
            f.write(cil.c_str(), (std::streamsize) cil.size());
        }
        else
        {
            Tailslide::LSOScriptCompiler visitor(&parser.allocator);
            script->visit(&visitor);
            if (visitor.mScriptBS.empty())
                return 1;
            std::ofstream f("/tmp/whatever.lso2", std::ios::out | std::ios::binary);
            f.write((const char *) visitor.mScriptBS.data(), (std::streamsize) visitor.mScriptBS.size());
        }
    }
    else
    {
        const char *outfile = "/tmp/whatever.lso2";
        if (compile_cil)
            outfile = "/tmp/whatever.cil";
        if (!lscript_compile(argv[1], outfile, "/dev/stderr", compile_cil, script_id.asString().c_str(), 0))
        {
            return 1;
        }
    }

    if (compile_cil)
        fprintf(stderr, "file written to /tmp/whatever.cil\n");
    else
        lscript_run("/tmp/whatever.lso2", FALSE);
    return 0;
}
