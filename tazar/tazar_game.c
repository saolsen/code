DPos dpos_from_cpos(CPos cpos) {
    int x = 2 * cpos.q + cpos.r;
    int y = cpos.r;
    return (DPos) {x, y};
}

CPos cpos_from_dpos(DPos dpos) {
    int q = (dpos.x - dpos.y) / 2;
    int r = dpos.y;
    int s = -q - r;
    return (CPos) {q, r, s};
}


CPos cpos_add(CPos a, CPos b) {
    return (CPos) {a.q + b.q, a.r + b.r, a.s + b.s};
}