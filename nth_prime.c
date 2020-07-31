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
#define LLVM_INT64_ZERO (LLVMConstInt(LLVMInt64Type(), 0L, 0))
#define LLVM_INT64_ONE (LLVMConstInt(LLVMInt64Type(), 1L, 0))
#define LLVM_INT64_TWO (LLVMConstInt(LLVMInt64Type(), 2L, 0))
#define LLVMBuildInc(builder, value, Name)                                                         \
    (LLVMBuildAdd(builder, value, LLVMConstInt(LLVMTypeOf(value), 1, 0), Name))
/*
  LLVM function to implement
  nth_prime(int n)
  {
    int i=1,j,count=0;
    while(i>0)
    {
        i++;j=2;
        while(j<i)
        {
            if(i%j==0)
                break;
            j++;
        }
        if(j>=i)
            count++;
        if(count==n)
            return i;
    }
  }

 */

int main(int argc, char const *argv[])
{
	LLVMInitializeNativeTarget();
	LLVMInitializeNativeAsmParser(); 
	LLVMInitializeNativeAsmPrinter();
	LLVMLinkInMCJIT(); 

	LLVMModuleRef module = LLVMModuleCreateWithName("nth_prime"); 
    //Creating the module under which all the functions will be bundled.    

	LLVMTypeRef params[] = {LLVMInt64Type()};
    //Defining input types to the function.

	LLVMTypeRef ret_type  = LLVMFunctionType(LLVMInt64Type(), params, 1,0);
    //Defining the llvm function type with return and number of input parameters

	LLVMValueRef nth_prime =  LLVMAddFunction(module, "nth_prime",ret_type);
    //Adding function with the defined function type to module

    LLVMBuilderRef builder = LLVMCreateBuilder();
	LLVMBasicBlockRef entry   = LLVMAppendBasicBlock(nth_prime,"entry");
    //Appending the function to a basic block

    LLVMBasicBlockRef if_div = LLVMAppendBasicBlock(nth_prime, "if_div");
    //if block to check the number is divisible

	LLVMBasicBlockRef if_ncount = LLVMAppendBasicBlock(nth_prime, "if_ncount");
    //if block to check the count reaches nth value

    LLVMBasicBlockRef for_ncount = LLVMAppendBasicBlock(nth_prime, "for_ncount");
    //for block to check the number is prime

    LLVMBasicBlockRef for_div_inc = LLVMAppendBasicBlock(nth_prime, "for_div_inc");
    //for block to go upto nth value

    LLVMBasicBlockRef for_exit = LLVMAppendBasicBlock(nth_prime, "exit");
    //exit block to return the nth prime number

	//move the position of builder to the end of the basic block and start adding code to block.
    LLVMPositionBuilderAtEnd(builder, entry);
    LLVMValueRef n = LLVMGetParam(nth_prime, 0);//Get n as input
    LLVMValueRef count = LLVMBuildAlloca(builder, LLVMInt64Type(), "");
    LLVMBuildStore(builder, LLVM_INT64_ZERO, count);//Initialize count value to count prime numbers
    LLVMValueRef loop__i = LLVMBuildAlloca(builder, LLVMInt64Type(), "");
    LLVMBuildStore(builder, LLVM_INT64_ONE, loop__i);//Initialize loop outer index 
    LLVMBuildBr(builder,for_ncount);

    //block to get a number 
    LLVMPositionBuilderAtEnd(builder, for_ncount);
    //Increment outer loop index to get a number
    LLVMValueRef loop_index_i = LLVMBuildLoad(builder, loop__i, "");
    loop_index_i = LLVMBuildInc(builder, loop_index_i, "");
    LLVMBuildStore(builder, loop_index_i, loop__i);
    //Initialize loop inner index 
    LLVMValueRef loop__j = LLVMBuildAlloca(builder, LLVMInt64Type(), "");
    LLVMBuildStore(builder, LLVM_INT64_TWO, loop__j);
    //Check j<i
    LLVMValueRef for_condition_2 = LLVMBuildICmp(builder, LLVMIntULT,LLVMBuildLoad(builder, loop__j, "") ,loop_index_i, "");
    LLVMBuildCondBr(builder, for_condition_2,if_div , if_ncount); 

    //block to increment the inner loop index ie:j
    LLVMPositionBuilderAtEnd(builder, for_div_inc);
    //Increment inner loop index 
    LLVMValueRef loop_index_j = LLVMBuildLoad(builder, loop__j, "");
    loop_index_j = LLVMBuildInc(builder, loop_index_j, ""); 
    LLVMBuildStore(builder, loop_index_j, loop__j);
    //Check j<i
    LLVMValueRef for_condition_3 = LLVMBuildICmp(builder, LLVMIntULT, loop_index_j,loop_index_i, "");
    LLVMBuildCondBr(builder, for_condition_3,if_div , if_ncount); 
    
    //block to check the number is divisible
    LLVMPositionBuilderAtEnd(builder, if_div);
    LLVMValueRef remainder = LLVMBuildURem(builder,loop_index_i,LLVMBuildLoad(builder, loop__j, ""), "");
    LLVMValueRef divisible = LLVMBuildICmp(builder, LLVMIntEQ, remainder, LLVM_INT64_ZERO, "");
    LLVMBuildCondBr(builder, divisible,for_ncount ,for_div_inc); 

    //block to check the count reaches the n
    LLVMPositionBuilderAtEnd(builder, if_ncount);
    LLVMValueRef count1 = LLVMBuildLoad(builder, count, "");
    count1 = LLVMBuildInc(builder, count1, "");
    LLVMBuildStore(builder, count1, count);
    LLVMValueRef count_condition= LLVMBuildICmp(builder, LLVMIntEQ, count1,n, "");
    LLVMBuildCondBr(builder, count_condition,for_exit , for_ncount);
 
    //block to return the nth prime number
    LLVMPositionBuilderAtEnd(builder, for_exit);
    LLVMBuildRet(builder, LLVMBuildLoad(builder, loop__i, ""));

    char	*error = NULL;
	LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
	LLVMDisposeMessage(error);

	LLVMExecutionEngineRef engine;
	error = NULL;
	LLVMCreateMCJITCompilerForModule(&engine, module, NULL, 0, &error);
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

	int (*fn_ptr)(int) = (int (*)(int)) LLVMGetPointerToGlobal(engine, nth_prime);

	printf("%d\n", fn_ptr(x));

	// Write out bitcode to file
	if (LLVMWriteBitcodeToFile(module, "nth_prime.bc") != 0)
	{
		fprintf(stderr, "error writing bitcode to file, skipping\n");
	}

	LLVMDisposeBuilder(builder);
	LLVMDisposeExecutionEngine(engine);
	return 0;
}