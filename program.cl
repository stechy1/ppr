__kernel void moje_hledani_extremu(
        __global double* mat, __global double* res,
        const int pocet_prvku,
        __local double* reduction_extremes) {
    const int globalID = get_global_id(0);
    const int localID = get_local_id(0);
    const int localSize = get_local_size(0);
    const int workgroupID = globalID / localSize;

    reduction_extremes[localID] = mat[globalID];

    for (int offset = localSize / 2; offset > 0; offset /= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);    // wait for all other work-items to finish previous iteration.
        if (localID < offset) {
            if (mat[localID] < reduction_extremes[0]) { reduction_extremes[0] = mat[localID]; }
            if (mat[localID] > reduction_extremes[1]) { reduction_extremes[1] = mat[localID]; }
        }
    }

    if (localID == 0) {    // the root of the reduction subtree
        res[workgroupID] = reduction_extremes[0];
    }
}