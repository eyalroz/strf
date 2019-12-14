#include <stdio.h> // for CUDA's printf

// #include "lightweight_test_label.hpp"
#include "test_utils.hpp"
#include <strf.hpp>
#include <sstream>

// Note: There are adaptations of, say, std::span for use with CUDA (= I adapted it...).
// But we want to avoid dependency clutter here, so let's just stick to the basics.


__global__ void kernel_using_cstr_writer(strf::cstr_writer::result* write_result, char* buffer, std::size_t buffer_size)
{
//  int global_thread_id = threadIdx.x + blockIdx.x * blockDim.x;
//  strf::snprintf(buf, "Thread %d says: Hello %s\n", global_thread_id, "world.");
//  printf("Thread %3d says: Hello %s\n", global_thread_id, "world.");
  strf::basic_cstr_writer<char> sw(buffer, buffer_size);
  write(sw, "Hello");
  write(sw, " world");
  *write_result = sw.finish();

//  if (not write_result->truncated) {
//	  printf("[%s kernel, thread %03d] Finalized string is: \"%s\"\n", __FUNCTION__, global_thread_id, buffer);
//  }
//  else {
//	  printf("[%s kernel, thread %03d] Finalized string is: \"%11s\"\n", __FUNCTION__, global_thread_id, buffer);
//  }
}

//__global__ void kernel_using_cstr_to(char* buffer, std::size_t buffer_size)
//{
//  int global_thread_id = threadIdx.x + blockIdx.x * blockDim.x;
//  auto printer = strf::to(buffer, buffer_size);
//  printer ( "Hello", ' ', "world, from thread ", global_thread_id );
//}



// Ugly, no-good error-checking.
#define ensure_cuda_success(ans) { ensure_cuda_success_((ans), __FILE__, __LINE__); }

inline void ensure_cuda_success_(cudaError_t status, const char *file, int line, bool abort=true)
{
  BOOST_TEST_EQ(status, cudaSuccess);
  if (abort and (status != cudaSuccess)) {
    BOOST_ERROR(cudaGetErrorString(status));
    exit(boost::report_errors());
  }
}


void test_cstr_writer()
{
	struct args {
		strf::cstr_writer::result write_result;
		char buffer[50];
	};
	const std::size_t buffer_size { std::strlen("Hello world") + 1 }; // Enough for "Hello world" with the trailing '\0'.
	struct args* device_side_args;
	ensure_cuda_success(cudaMalloc(&device_side_args, sizeof(struct args)));
	ensure_cuda_success(cudaMemset(device_side_args, 0, sizeof(struct args)));

	int threads_per_block { 1 };
	int blocks_in_grid { 1 };
		// We could theoretically have multiple threads in multiple blocks run this, but
		// it shouldn't really matter.
	kernel_using_cstr_writer<<<threads_per_block, blocks_in_grid>>>(
		&(device_side_args->write_result),
		&(device_side_args->buffer[0]),
		buffer_size);
	ensure_cuda_success(cudaGetLastError());
	ensure_cuda_success(cudaDeviceSynchronize());
	args host_side_args;
	ensure_cuda_success(cudaMemcpy(&host_side_args, device_side_args, sizeof(struct args), cudaMemcpyDeviceToHost));
    BOOST_TEST_EQ(host_side_args.write_result.truncated, false);
	BOOST_TEST_EQ(host_side_args.write_result.ptr, &(device_side_args->buffer[0]) + std::strlen("Hello world"));
	if (host_side_args.write_result.ptr == &(device_side_args->buffer[0])) {
		BOOST_TEST_EQ(strncmp(host_side_args.write_result.ptr, host_side_args.buffer, buffer_size), 0);
	}
}

/*
void test_cstr_to()
{
	char* device_side_buffer;
	const std::size_t buffer_size { 100 }; // More than enough for "Hello world from thread XYZ"
	ensure_cuda_success( cudaMalloc(&device_side_buffer, buffer_size) );
	ensure_cuda_success( cudaMemset(device_side_buffer, 0, buffer_size) );

	int threads_per_block { 1 };
	int blocks_in_grid { 1 };
		// We could theoretically have multiple threads in multiple blocks run this, but
		// it shouldn't really matter.
	kernel_using_cstr_to<<<threads_per_block, blocks_in_grid>>>(device_side_buffer, buffer_size);
	ensure_cuda_success(cudaGetLastError());
	ensure_cuda_success(cudaDeviceSynchronize());
	char host_side_buffer[buffer_size];
	ensure_cuda_success(cudaMemcpy(&host_side_buffer, device_side_buffer, buffer_size , cudaMemcpyDeviceToHost));
	std::stringstream expected;
	expected << "Hello" << ' ' << "world, from thread " << 1;
	BOOST_TEST_EQ(strncmp(host_side_buffer, expected.str().c_str(), buffer_size), 0);
}
*/

void cstr_to_sanity_check()
{
	const std::size_t buffer_size { 100 }; // More than enough for "Hello world from thread XYZ"
	char buffer[buffer_size];
	std::fill_n(buffer, sizeof(buffer), 0);
    auto print_functor = strf::to(buffer, buffer_size);
    print_functor ( "Hello", ' ', "world, from thread ", 1 );
	std::stringstream expected;
	expected << "Hello" << ' ' << "world, from thread " << 1;
	BOOST_TEST_EQ(strncmp(buffer, expected.str().c_str(), buffer_size), 0);
}


int main(void)
{
    auto num_devices { 0 };
    auto status = cudaGetDeviceCount(&num_devices);

    BOOST_TEST_EQ(status, cudaSuccess);
    if (status != cudaSuccess)
    {
    	std::stringstream ss;
    	ss << "cudaGetDeviceCount failed: " << cudaGetErrorString(status) <<  '\n';
        BOOST_ERROR(ss.str().c_str());
    }
    if (num_devices == 0) {
        std::cerr << "No devices - can't run this test\n";
        return boost::report_errors();
    }
	// TODO: Test basic_cstr_writer's with different character types
	test_cstr_writer();
	cstr_to_sanity_check();
//	test_cstr_to();

    cudaDeviceReset();
    return boost::report_errors();
}
