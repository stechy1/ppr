__kernel void moje_hledani_extremu(
    __global double* mat, __global double* res,
    const int pocet_prvku) {
    const int idx = get_global_id(0);
    if (idx < pocet_prvku) {
        if (mat[idx] < res[0]) { res[0] = mat[idx]; }
        if (mat[idx] > res[1]) { res[1] = mat[idx]; }
    }
}