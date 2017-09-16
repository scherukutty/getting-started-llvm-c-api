#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Transforms/IPO.h>
#include <llvm-c/Transforms/PassManagerBuilder.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
/*
  LLVM function to implement
  factorial(int n)
  {
  	if(n<=0)
		return 1;
	else
		return n*factorial(n-1);
  }

 */
int main(int argc, char const *argv[])
{
	LLVMInitializeNativeTarget(); // Initializing target
	LLVMInitializeNativeAsmParser(); // Init Native assembly Parser and printer
	LLVMInitializeNativeAsmPrinter();
	LLVMLinkInMCJIT(); // Make the program link via mcjit

	//creating module
	LLVMModuleRef module = LLVMModuleCreateWithName("factorial"); //Creating the big module.
	//Everything is a part of the module.
	LLVMBuilderRef builder = LLVMCreateBuilder();// Creating the builder for generating the code.

	//setting input parameters and return type of function
	LLVMTypeRef params[] = {LLVMInt64Type()};
	LLVMTypeRef ret_type  = LLVMFunctionType(LLVMInt64Type(), params, 1,
											 0); //Initializing the return types. Input and output.

	//adding the function to module
	LLVMValueRef factorial =  LLVMAddFunction(module, "factorial",
											  ret_type);// Adding a function to module

	//append basic block.. basic block is the smallest entity of block based execution.
	LLVMBasicBlockRef entry   = LLVMAppendBasicBlock(factorial,
													 "entry"); //Entry block get the value and does comparison
	//and branching if n <= 0
	LLVMBasicBlockRef ifblock = LLVMAppendBasicBlock(factorial, "if");// ifblock return 1
	LLVMBasicBlockRef elseblock = LLVMAppendBasicBlock(factorial,
													   "else"); //elseblock return n*factorial(n-1)

	//move the position of builder to the end of the basic block and start adding code to the block.
	LLVMPositionBuilderAtEnd(builder, entry);
	LLVMValueRef n = LLVMGetParam(factorial, 0);//get n from the function factorial
	LLVMValueRef nminusone = LLVMBuildSub(builder, n, LLVMConstInt(LLVMInt64Type(), 1L, 0),
										  "nminus1");//get n-1
	LLVMValueRef cmp = LLVMBuildICmp(builder, LLVMIntSLE, n, LLVMConstInt(LLVMInt64Type(), 0L, 0),
									 " compare n with zero");//make comparision n<=0
	LLVMBuildCondBr(builder, cmp, ifblock, elseblock);//Make the conditional branch with the condition

	LLVMPositionBuilderAtEnd(builder, ifblock);
	LLVMBuildRet(builder, LLVMConstInt(LLVMInt64Type(), 1L, 0)); //return 1 if comes to if block

	LLVMPositionBuilderAtEnd(builder, elseblock);
	LLVMValueRef args[] = {nminusone};
	LLVMValueRef res = LLVMBuildMul(builder, n, LLVMBuildCall(builder, factorial, args, 1,
															  "recursive call"), "multiplication"); //returns n*factorial(n-1)
	LLVMBuildRet(builder, res);

	//done writing the function

	//setting up for execution
	char	*error = NULL;
	LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
	//module verification
	LLVMDisposeMessage(error);

	LLVMExecutionEngineRef engine;
	error = NULL;
	LLVMCreateMCJITCompilerForModule(&engine, module, NULL, 0, &error);
	//Setting up mcjit compiler for module
	LLVMDisposeMessage(error);

	error = NULL;

	if (LLVMCreateExecutionEngineForModule(&engine, module, &error) != 0)
	{
		fprintf(stderr, "failed to create execution engine\n");
		abort();
	}

	if (error)
	{
		fprintf(stderr, "error: %s\n", error);
		LLVMDisposeMessage(error);
		exit(EXIT_FAILURE);
	}

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s x y\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	long long x = strtoll(argv[1], NULL, 10);

	int (*fn_ptr)(int) = (int (*)(int)) LLVMGetPointerToGlobal(engine, factorial);

	printf("%d\n", fn_ptr(x));

	// Write out bitcode to file
	if (LLVMWriteBitcodeToFile(module, "factorial.bc") != 0)
	{
		fprintf(stderr, "error writing bitcode to file, skipping\n");
	}

	LLVMDisposeBuilder(builder);
	LLVMDisposeExecutionEngine(engine);
	return 0;
}
