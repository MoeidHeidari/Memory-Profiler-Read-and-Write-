
//=====================================================================================

#include <iostream>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#define INCLUDE_ALGORITHM
#define INCLUDE_FUNCTIONAL
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "rtl.h"
#include "df.h"
#include "cfgloop.h"
// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;
static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a very simple plugin" };
bool findCode(rtx expr, rtx_code code){
	if( expr == 0x00 ){
		return false;
	}

	rtx_code exprCode = (rtx_code) expr->code;			// Get the code of the expression
	const char* format = GET_RTX_FORMAT(exprCode);		// Get the format of the expression, tells what operands are expected

	if(exprCode == code){					// Test if expression is a CODE expression
		return true;
	}
	else if(exprCode == ASM_OPERANDS){
		return false;
	}
	else{
		for (int x=0; x < GET_RTX_LENGTH(exprCode); x++){	// Loop over all characters in the format
			if(format[x] == 'e'){							// Test if they are an expression
				rtx subExpr = XEXP(expr,x);					// Get the expression
				if (findCode(subExpr, code)){				// Recursive call to this function
					return true;
				}
			}
			else if(format[x] == 'E'){						// Test if a Vector
				for(int i=0; i<XVECLEN(expr,0);i++){		// Loop over all expressions in the vector
					rtx subExpr = XVECEXP(expr, 0, i);		// Get the expression
					if(findCode(subExpr, code)){			// Recursive call to this function
						return true;
					}
				}
			}
		}
	}
	return false;
}
rtx_insn* findInsn(basic_block bb, unsigned int index){
	rtx_insn* insn = BB_HEAD(bb);
	int count = 0;
	while(count < index){
		insn = NEXT_INSN(insn);
		if(NONDEBUG_INSN_P(insn)){
			count++;
		}
	}
	return insn;
}
vec<rtx_insn*>* find_mem_ins_in_bb(basic_block bb)
{
  vec<rtx_insn*>* indeices;
  rtx_insn* insn = BB_HEAD(bb);
	int count = 0;
	while(insn){
		insn = NEXT_INSN(insn);
		if(NONDEBUG_INSN_P(insn)){
			if(contains_mem_rtx_p(insn))
      {
        indeices->safe_push(insn);

      }
		}
	}
  return indeices;
}
//==============================================================================

void printRTL(char* fileName){
  	FILE *fp = fopen(fileName, "w");
  	if (fp != NULL){
  		basic_block bb;
  		FOR_ALL_BB_FN(bb, cfun){					// Loop over all Basic Blocks in the function, cfun = current function
  			fprintf(fp,"BB: %d\n", bb->index-2);
  			rtx_insn* insn;
  			FOR_BB_INSNS(bb, insn){
  				// Loop over all rtx statements in the Basick Block
  				if( NONDEBUG_INSN_P(insn) ){		// Filter all actual code statements
  					//print_simple_rtl(fp, insn);
  					print_rtl_single(fp, insn);		// print to file
  				}
  			}
  			fprintf(fp,"\n----------------------------------------------------------------\n\n");
  		}
  		fclose(fp);
  	}
  }
//------------------------------------------------
void printEdges(char* fileName){
	FILE *fp = fopen(fileName, "w");
	if (fp != NULL){
		basic_block bb;
		FOR_EACH_BB_FN(bb, cfun){
			unsigned int idBB = bb->index-2;
			fprintf(fp, "BB: %d\n", idBB);
			edge e;
			edge_iterator ei;
			char counter = 0;
			FOR_EACH_EDGE(e, ei, bb->succs){
				counter++;
			}
			unsigned int idDests[counter];
			char index = 0;
			FOR_EACH_EDGE(e, ei, bb->succs){
				unsigned int idDest = (e->dest)->index -2;
				idDests[index] = idDest;
				index ++;
			}
			if(counter == 2){
				if(idDests[1] < idDests[0]){
					unsigned int temp  = idDests[0];
					idDests[0] = idDests[1];
					idDests[1] = temp;
				}
			}
			for(char x = 0; x < counter; x++){
				fprintf(fp, "\t%i --> %i\n", idBB, idDests[x]);
			}
			fprintf(fp,"\n");
		}
		fclose(fp);
	}
}
//------------------------------------------------
void blockAnalysis(char* fileName){
	unsigned int totalBlocks = n_basic_blocks_for_fn(cfun)-2;

	FILE* fp = fopen(fileName, "a");
	if (fp != NULL){
		fprintf(fp, "Block Analysis:\n");
		fprintf(fp, "\tTotal amount of basic blocks: %i\n", totalBlocks);

		basic_block bb;
		FOR_EACH_BB_FN(bb, cfun){
			unsigned int index = bb->index -2;
			unsigned int length = 0;
			rtx_insn* insn;
			FOR_BB_INSNS(bb, insn){
				if(NONDEBUG_INSN_P(insn)){
					length++;
				}
			}
			fprintf(fp, "\tLength of basic block %i: %i\n", index, length);
		}
		fclose(fp);
	}
}
//------------------------------------------------
void edgeAnalysis(char* fileName){
	// Variables to analyze the edges current function
	unsigned int totalEdges = 0;
	unsigned int uncondEdges = 0;
	basic_block bb;
	FOR_EACH_BB_FN(bb, cfun){
		edge e;
		edge_iterator ei;
		char counter = 0;
		FOR_EACH_EDGE(e, ei, bb->succs){
			totalEdges++;
			counter++;
		}
		if (counter == 1){
			uncondEdges ++;
		}
	}
	unsigned int condEdges = totalEdges - uncondEdges;

	// Print result of edge analysis
	FILE* fp = fopen(fileName, "w");
	if( fp != NULL){
		fprintf(fp, "Edge Analysis:\n");
		fprintf(fp, "\tTotal amount of edges: %i\n", totalEdges);
		fprintf(fp, "\tNumber of unconditional edges: %i\n", uncondEdges);
		fprintf(fp, "\tNumber of conditional edges: %i\n", condEdges);
		fprintf(fp, "------------------------------------------------------\n");
		fclose(fp);
	}
}

void printAnalysis(char* fileName){
	edgeAnalysis(fileName);
	blockAnalysis(fileName);
}
//==============================================================================
namespace
{
  const struct pass_data _pass_data =
{
  .type = RTL_PASS,
  .name = "memory_counter_plugin",
  .optinfo_flags = OPTGROUP_NONE,
  .tv_id = TV_TREE_CLEANUP_CFG,
  .properties_required = 0,//(PROP_rtl | PROP_cfglayout),
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0,
};


    struct memory_counter_pass : rtl_opt_pass
    {
        memory_counter_pass(gcc::context *ctx)
            : rtl_opt_pass(_pass_data, ctx)
        {
        }
        bool gate(function *fun)
        {
          return true;
        }
        unsigned int execute(function *fun)
        {
          printEdges((char *)"Edges.txt");
        printAnalysis((char *)"Analysis.txt");
          char* funName = (char*)IDENTIFIER_POINTER (DECL_NAME (current_function_decl) );
          printf("function detected-->%s\n",funName);
          basic_block bb;
          int counter=0;
          FOR_EACH_BB_FN(bb, cfun)
          {
            const rtx_insn* insn;
            FOR_BB_INSNS(bb, insn)
            {
              if(INSN_HAS_LOCATION(insn))
              {
                printf("%s\n","symbol");
              }

              //printf("%d\n",INSN_CODE(insn) );
                  //rtx innerExpr = XEXP(insn, 3);
                  if(GET_RTX_CLASS (INSN_CODE(insn)) == RTX_COMM_ARITH  && MEM_EXPR(insn))
                  {

                  }
            }

            counter++;



            // unsigned int idBB = (bb->index) - 2;
            // rtx_insn* insn;
            // FOR_BB_INSNS(bb, insn){
      			// 	if( NONDEBUG_INSN_P(insn) )
            //   {		// Filter all actual code statements
      			// 		//print_simple_rtl(fp, insn);
      			// 		//print_rtl_single(fp, insn);		// print to file
            //     rtx innerExpr = XEXP(insn, 3);
	          //     if(MEM_EXPR (innerExpr))
            //     {
            //       printf("memory detected\n");
            //     }
      			// 	}
      			// }
            //


        }
            return 0;
        }

    };
}

int plugin_init (struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version)
{
	// We check the current gcc loading this plugin against the gcc we used to
	// created this plugin
	if (!plugin_default_version_check (version, &gcc_version))
    {
        std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
    }

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &my_gcc_plugin_info);

    // Register the phase right after omplower
    struct register_pass_info pass;

    // Note that after the cfg is built, fun->gimple_body is not accessible
    // anymore so we run this pass just before the cfg one

	pass.pass = new memory_counter_pass(g);
	pass.reference_pass_name = "*free_cfg";
	pass.ref_pass_instance_number = 1;
	pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);

    return 0;
}
