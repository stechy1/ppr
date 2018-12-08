__kernel void normalize(){
		__global const double* mat,
		// __global double* max,
		// __global double* min,
		// const int num) {
	// const int globalID = get_global_id(0);
	// const int localID = get_local_id(0);
	// const int localSize = get_local_size(0);
	// const int workgroupID = globalID / localSize;

	// for(int offset = localSize / 2; offset > 0; offset /= 2) {
	// 	barrier(CLK_LOCAL_MEM_FENCE);	// wait for all other work-items to finish previous iteration.
	// 	if(localID < offset) {
	// 		double val = mat[localID];
	// 		if (val > *max) {
	// 			*max = val;
	// 		}
	// 		if (val < *min) {
	// 			*min = val;
	// 		}
	// 	}
	// }
}