__kernel void moje_hledani_extremu(
        __global const double* mat,
        __global double* res,
        const int pocet_prvku,
        __local double* reduction_min,
        __local double* reduction_max
) {
    const int globalID = get_global_id(0);
    const int localID = get_local_id(0);
    const int localSize = get_local_size(0);
    const int workgroupID = globalID / localSize;

    res[localID] = reduction_min[localID] =
    reduction_max[localID] = mat[localID];

    for (int offset = localSize / 2; offset > 0; offset /= 2) {
        barrier(CLK_LOCAL_MEM_FENCE);
        int curr = localID + offset;
        if (localID < offset) {
            if (reduction_min[localID] > reduction_min[curr]) {
                reduction_min[localID] = reduction_min[curr];
            }
            if (reduction_max[localID] < reduction_max[curr]) {
                reduction_max[localID] = reduction_max[curr];
            }
        }
    }
    if (localID < pocet_prvku) {
        res[localID] = (res[localID] - reduction_min[0]) /
                       (reduction_max[0] - reduction_min[0]);
    }
}