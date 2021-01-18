#include <iostream>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"
#include <print-tree.h>
#include <c-family/c-common.h>
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
#include "cfgloop.h"

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;
uint64_t loop_niter;
int func_memory_reader=2;
static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a very simple plugin" };
basic_block mem_blocks[2];
location_t l;
int read_counter=0;
int write_counter=0;
bool isVisited;
namespace
{
    const pass_data Mem_Pass_Before =
    {
        GIMPLE_PASS,
        "mem_pass_before",        /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };
//==============================================================================================================
    const pass_data Mem_Pass_After=
    {
        GIMPLE_PASS,
        "mem_pass_after",        /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct mem_op_pass_before : gimple_opt_pass
    {

        mem_op_pass_before(gcc::context *ctx)
            : gimple_opt_pass(Mem_Pass_Before, ctx)
        {
        }

        virtual unsigned int execute(function *fun) override
        {
            // fun is the current function being called
            gimple_seq gimple_body = fun->gimple_body;
            struct walk_stmt_info walk_stmt_info;
            memset(&walk_stmt_info, 0, sizeof(walk_stmt_info));
            walk_gimple_seq(gimple_body, callback_stmt, callback_op, &walk_stmt_info);
            // Nothing special todo
            return 0;
        }
        virtual mem_op_pass_before* clone() override
        {
            // We do not clone ourselves
            return this;
        }
        private:
        static tree callback_stmt(gimple_stmt_iterator * gsi, bool *handled_all_ops, struct walk_stmt_info *wi)
        {
             gimple* g = gsi_stmt(*gsi);
            enum gimple_code code = gimple_code(g);
            if(gimple_has_mem_ops(g))
            {
              if(gimple_assign_rhs_code(g) && TREE_CODE(gimple_assign_lhs(g))==MEM_REF)
              {
                 l = gimple_location(g);
                  printf("memory write\n");
                  isVisited=true;
                  write_counter++;
              }
            }
            return NULL;
        }
        static tree callback_op(tree *t, int *, void *data)
        {
            enum tree_code code = TREE_CODE(*t);
            if(code==MEM_REF)
            {
              if(func_memory_reader <=0)
              {
                printf("memory read\n");
                read_counter++;
              }
              else
              {
                func_memory_reader--;
              }
            }
            return NULL;
        }
    };
//==========================================================================================================
struct mem_op_pass_after : gimple_opt_pass
{
    mem_op_pass_after(gcc::context *ctx)
        : gimple_opt_pass(Mem_Pass_After, ctx)
    {
    }

    virtual unsigned int execute(function *fun) override
    {
      //printf("%d\n", number_of_loops(cfun));
      //printf("%d\n",vec_safe_length(mem_blocks));
      //get_loops(fun)->pop();
      //loop_niter=get_simple_loop_desc(get_loops(fun)->pop())->niter;
          //std::cout<< get_simple_loop_desc(get_loops(fun)->pop())->niter<<std::endl;
          // if(loops)
          // {
          //   // class niter_desc* loop_niter=simple_loop_desc(cloop);
          //   // printf("%ld\n",loop_niter->niter);
          // }

        return 0;
    }
    virtual mem_op_pass_after* clone() override
    {
        // We do not clone ourselves
        return this;
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
    struct register_pass_info pass_info_after;
    struct register_pass_info pass_info_before;

    // Note that after the cfg is built, fun->gimple_body is not accessible
    // anymore so we run this pass just before the cfg one
    pass_info_after.pass = new mem_op_pass_after(g);
    pass_info_after.reference_pass_name = "cfg";
    pass_info_after.ref_pass_instance_number = 1;
    pass_info_after.pos_op = PASS_POS_INSERT_AFTER;
//===========================================================================================================
    pass_info_before.pass = new mem_op_pass_before(g);
    pass_info_before.reference_pass_name = "cfg";
    pass_info_before.ref_pass_instance_number = 1;
    pass_info_before.pos_op = PASS_POS_INSERT_BEFORE;
    std::cout<<loop_niter<<std::endl;
    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info_before);
    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info_after);

    return 0;
}
