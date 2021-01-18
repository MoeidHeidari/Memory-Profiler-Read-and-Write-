
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
 #include "llvm/Pass.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
 #include "llvm/Analysis/LoopAccessAnalysis.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/Statistic.h"
#include <map>
using namespace llvm;

namespace {
struct MemoryPass : public PassInfoMixin<MemoryPass>
{
  int memory_counter=0;
  std::map<std::string, int> opCounter;
  void getAnalysisUsage(AnalysisUsage &AU) const{
    AU.setPreservesAll();
    AU.addRequired<LoopInfoWrapperPass>();

  }

  bool runOnFunction(Function &F) {

    return false;
  }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
  {

    for (BasicBlock &BB : F)
    {



      for (Instruction &Instr : BB) {
          opCounter[Instr.getOpcodeName()] += 1;
          if (isa<LoadInst>(Instr))
          {
            opCounter["Memory loads"] += 1;
         }
         else if (isa<StoreInst>(Instr))
         {

          opCounter["Memory writes"] += 1;

        }
      }
    }

    llvm::outs()<<"Function::"<<F.getName()<<"----------------------------------------------------------\n";
    std::map <std::string, int>::iterator i = opCounter.begin();
   std::map <std::string, int>::iterator e = opCounter.end();
   while (i != e) {
   errs() << i->first << ": " << i->second << "\n";
   i++;
   }
   errs() << "\n";
   opCounter.clear();

    return PreservedAnalyses::all();
  }
};
}  // end anonymous namespace


extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "myplugin", "v0.1", [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "myplugin") {
                    FPM.addPass(MemoryPass());
                    return true;
                  }
                  return false;
                });
          }};
}
