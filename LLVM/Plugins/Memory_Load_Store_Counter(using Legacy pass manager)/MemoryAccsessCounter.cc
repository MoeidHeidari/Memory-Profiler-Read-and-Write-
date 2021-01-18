// LLVM  Libraries
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
//STL libraries
#include <vector>
using namespace llvm;
//=================================HEADER=======================================
NameSpace
{
  class Memory_access_counter : public ModulePass
  {
//**********************************PUBLIC**************************************
  public:
    Memory_access_counter()
//------------------------------------------------------------------------------
    virtual bool runOnModule(Module &module);
//------------------------------------------------------------------------------
    void init_engine(Module &);
//------------------------------------------------------------------------------
    void begin_profiling(Module &);
//------------------------------------------------------------------------------
    void print_the_result(int &,int &);

//**********************************PRIVATE**************************************
  private:
    static char                 m_Pass_id=0;
    IRBuilder<>*                m_ir_builder;
    std::vector<Constant *>     m_entry_points;
    Constant*                   m_null_value;
    Constant*                   m_load_str_ptr;
    Constant*                   m_write_str_ptr;
    int                         m_mem_load_counter;
    int                         m_mem_write_counter;

    Function*                   m_print_func;
    Constant*                   m_load_string;
    Constant*                   m_Write_string;
    GlobalVariable*             m_load_str_var;
    GlobalVariable*             m_write_str_var;
    char*                       m_addr_load_str;
    char*                       m_addr_write_str;
    char*                       m_function_name;

};
//===============================IMPL===========================================
Memory_access_counter::Memory_access_counter():ModulePass(m_Pass_id){}
//------------------------------------------------------------------------------
void Memory_access_counter::init_engine(Module &MOD){
  m_addr_load_str    ="I got a load address";
//...............
  m_addr_write_str   ="I got a write address";
//...............
  m_ir_builder       =new IRBuilder<>(MOD.getContext());
//...............
  m_print_func       =Function::Create(TypeBuilder<int(char*, ...),
                      false>::get(MOD.getContext()),
                      Function::ExternalLinkage,"printf",&MOD);
//...............
  m_load_string      =ConstantDataArray::getString(MOD.getContext(),
                     m_addr_load_str);
//...............
  m_Write_string     =ConstantDataArray::getString(MOD.getContext(),
                     m_addr_write_str);
//...............
  m_load_str_var     =new GlobalVariable(MOD,m_load_string->getType(),true,
                     Function::InternalLinkage,m_load_string,"load str");
//...............
  m_write_str_var    =new GlobalVariable(MOD,m_Write_string->getType(),true,
                     Function::InternalLinkage,m_Write_string,"write str");
//...............
  m_null_value       =Constant::getNullValue(
                     IntegerType::getInt32Ty(MOD.getContext()));
//...............
  m_entry_points.push_back(m_null_value);
  m_entry_points.push_back(m_null_value);
//...............
  m_load_str_ptr     =ConstantExpr::getGateElementPtr(m_load_string->getType(),
                     m_load_str_var,m_entry_points);
//...............
  m_write_str_ptr    =ConstantExpr::getGateElementPtr(m_Write_string->getType(),
                     m_write_str_var,m_entry_points);

  m_mem_load_counter =0;
//...............
  m_mem_write_counter=0;
}

//------------------------------------------------------------------------------
virtual bool Memory_access_counter::runOnModule(Module &module)
{
  init_engine(module);
//...............................
  begin_profiling(module);
//...............................
  print_the_result(this->m_mem_load_counter,this->m_mem_write_counter);
//...............................
   return true;
}
void Memory_access_counter::begin_profiling(Module &MOD)
{
  for(Function &F : MOD) {
        for(BasicBlock &B : F) {
          for(Instruction &I : B)
          {
            m_function_name=F.getName();
            LoadInst * load = nullptr;
            StoreInst * store = nullptr;

            m_ir_builder->SetInsertPoint(&I);

            if((load = dyn_cast<LoadInst>(&I)) != nullptr) {
              m_mem_load_counter++;

              Value * address = load->getPointerOperand();

              std::vector<Value *> args;
              args.push_back(m_load_str_ptr);
              args.push_back(address);
              CallInst * call = m_ir_builder->CreateCall(
                print,
                args,
                ""
              );
            } else if ((store = dyn_cast<StoreInst>(&I)) != nullptr) {
              m_mem_write_counter++;

              Value * value = store->getValueOperand();
              Value * address = store->getPointerOperand();
              std::vector<Value *> args;
              args.push_back(m_write_str_ptr);
              args.push_back(value);
              args.push_back(address);


              CallInst * call = m_ir_builder->CreateCall(
                print,
                args,
                ""
              );
            }
          }
        }
      }

}
//------------------------------------------------------------------------------
void Memory_access_counter::print_the_result(int &loads,int &writes)
{
  errs() <<"*******************"<<<m_function_name<"*************************\n";
  errs() <<"**"<<loads<<"memory loads has beenn detected\n";
  errs() <<"**"<<writes<<"memory writes has beenn detected\n";
  errs() <<"-------------------------------------------------\n";
  errs() <<"**"<<"all together:"<<(loads + writes)<<"memory access detected\n";
}
}
register_Memory_access_counter_pass
    (const PassManagerBuilder pass_manager_builder&,
    legacy::PassManagerBase &legacy_pass_mmanager)
{
    legacy_pass_mmanager.add(new Memory_access_counter());
}
static RegisterStandardPasses RegisterMyPass(PassManagerBuilder::EP_EnabledOnOptLevel0, register_Memory_access_counter_pass);
