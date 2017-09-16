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


int main(int argc, char const *argv[])
{
	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmParser();
	LLVMInitializeNativeAsmPrinter();
	LLVMLinkInMCJIT();
	LLVMModuleRef	mod = LLVMModuleCreateWithName("sum_module");
	//Creating the module under which all the functions will be bundled.

	LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type() };
	//Defining input types to the function.

	LLVMTypeRef ret_type = LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0);
	//Defining the llvm function type with return and number of input parameters

	LLVMValueRef	sum = LLVMAddFunction(mod, "sum", ret_type);
	//Adding function with the defined function type to module

	LLVMBasicBlockRef	entry = LLVMAppendBasicBlock(sum, "entry");
	//Appending the function to a basic block

	LLVMBuilderRef	builder = LLVMCreateBuilder();
	LLVMPositionBuilderAtEnd(builder, entry);
	//Creating builder and postioning it to start writing instructions

	LLVMValueRef	tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
	//Takes the address of builder and parameters to return result

	LLVMBuildRet(builder, tmp);

	char *error = NULL;
	LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
	LLVMDisposeMessage(error);

	LLVMExecutionEngineRef engine;
	error = NULL;
	LLVMCreateMCJITCompilerForModule(&engine, mod, NULL, 0, &error);
	LLVMDisposeMessage(error);
	error = NULL;

	if (LLVMCreateExecutionEngineForModule(&engine, mod, &error) != 0)
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

	if (argc < 3)
	{
		fprintf(stderr, "usage: %s x y\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	long long x = strtoll(argv[1], NULL, 10);
	long long y = strtoll(argv[2], NULL, 10);

	int (*sum_fn)(int, int) = (int (*)(int, int)) LLVMGetPointerToGlobal(engine, sum);

	printf("%d\n", sum_fn(x, y));

	// Write out bitcode to file
	if (LLVMWriteBitcodeToFile(mod, "sum.bc") != 0)
	{
		fprintf(stderr, "error writing bitcode to file, skipping\n");
	}

	LLVMDisposeBuilder(builder);
	LLVMDisposeExecutionEngine(engine);
	return 0;
}
