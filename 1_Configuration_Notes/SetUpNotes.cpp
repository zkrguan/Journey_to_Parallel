// This is for first and second setup 
//--------------------------------------------//
//#include <iostream>
//// This will be library I will be working with for 6 weeks of study 
//#include <omp.h>
//
//int main(int argc, char** argv)
//{
//	// _OPENMP is just the MACRO for the version//
//	std::cout << "Hello C++, I am back." << _OPENMP;  // Different compiler will have different values. Intel is 2018. 
//}
//------------------------------------------//


//// This is for setting up TBB // 
////-------------------------------------//
//#include <iostream>
//#include <tbb/tbb.h>
//int main() {
//	std::cout << "Hello world from TBB" << TBB_VERSION_MAJOR << "|" << TBB_VERSION_MINOR << "|" << TBB_INTERFACE_VERSION << "|" << std::endl;
//}
////-------------------------------------//
 
// This is for setting up MPI from MS // 
//-------------------------------------//
#include <iostream>
#include <mpi.h>
// Don't forgot to send arguments
int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	std::cout << "Hello from MPI" << std::endl;
	MPI_Finalize();
}
//-------------------------------------//




/*
* For setup open MP
* 
* solution finder -> projName right click -> properties
* 
* 1. General -> language greater than C++ 17
* 
* 2. C++ -> language -> open MP support -> yes

*/


/*
* For setup intel compiler
* We will use oneAPI HPC Toolkit from Intel* 
* https://www.intel.com/content/www/us/en/developer/tools/oneapi/hpc-toolkit-download.html?operatingsystem=window&distributions=online
* 
* 1. Project (top banner) -> Intel Compiler -> choose the Intel compiler 
* 2. project -> right click -> properties -> C/C++ language -> [Intel C++] 1. OpenMP Support -> Parallel, 2. C/C++ language support C++17
*/


/*
* For setup thread building block (TBB) from Intel
* This is open source library
* 
* 1. Project -> right click -> properties -> configration properties -> intel library for one apis -> useOne TBB => YES
*/


// ICX - CL meaning we used intel compiler



/*
*	MPI, 
*		This is some kind of protocol for communications between processors. (think about using BullMQ or other things)
*		1. https://www.microsoft.com/en-US/download/details.aspx?id=57467
*		2. Make sure the window environment variables direct to the correct path to the MPI lib
*		3. Properties -> c++/c -> Additional include directories. -> path to the microsoft MPI include C:\Program Files %28x86%29\Microsoft SDKs\MPI\Include
*					  -> Linker -> general -> Additional lib  C:\Program Files %28x86%29\Microsoft SDKs\MPI\Lib\x64;%(AdditionalLibraryDirectories)
*								-> input ->  msmpi.lib;%(AdditionalDependencies)
*					  -> debugging -> command C:\Program Files (x86)\Intel\oneAPI\mpi\2021.10.0\bin\mpiexec.exe
*					  -> command arguments -n 8 "$(TargetPath)" 
*/					 // run 8 times 