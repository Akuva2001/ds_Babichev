struct context_Bully_s{
    bool started = false;
    bool finished = false;
    long long start_time = 0;
    int coordinator = -1;
    long long wait_delta = 50;
} contextBully;