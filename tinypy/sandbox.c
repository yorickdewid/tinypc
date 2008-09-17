void tp_sandbox(TP, double time_limit, size_t mem_limit) {
    tp->time_limit = time_limit;
    tp->mem_limit = mem_limit;
}

void tp_mem_update(TP) {
    if((!tp->mem_exceeded) &&
       (tp->mem_used > tp->mem_limit) && 
       (tp->mem_limit != TP_NO_LIMIT)) {
        tp->mem_exceeded = 1;
        tp_raise(,tp_string("(tp_mem_update) SandboxError: memory limit exceeded"));
    }
}

void tp_time_update(TP) {
    clock_t tmp = tp->clocks;
    if(tp->time_limit != TP_NO_LIMIT)
    {
        tp->clocks = clock();
        tp->time_elapsed += ((double) (tp->clocks - tmp) / CLOCKS_PER_SEC) * 1000.0;
        if(tp->time_elapsed >= tp->time_limit)
            tp_raise(,tp_string("(tp_time_update) SandboxError: time limit exceeded"));
    }
}

void *tp_malloc(TP, size_t bytes) {
    size_t *ptr = (size_t *) calloc(bytes + sizeof(size_t), 1);
    if(ptr) {
        *ptr = bytes;
        tp->mem_used += bytes + sizeof(size_t);
    }
    tp_mem_update(tp);
    return ptr+1;
}

void tp_free(TP, void *ptr) {
    size_t *temp = (size_t *) ptr;
    if(temp) {
        --temp;
        tp->mem_used -= (*temp + sizeof(size_t));
        free(temp);
    }
    tp_mem_update(tp);
}

void *tp_realloc(TP, void *ptr, size_t bytes) {
    size_t *temp = (size_t *) ptr;
    int diff;
    if(temp && bytes) {
        --temp;
        diff = bytes - *temp;
        *temp = bytes;
        tp->mem_used += diff;
        temp = (size_t *) realloc(temp, bytes+sizeof(size_t));
        return temp+1;
    }
    else if(temp && !bytes) {
        tp_free(tp, temp);
        return NULL;
    }
    else if(!temp && bytes) {
        return tp_malloc(tp, bytes);
    }
    else {
        return NULL;
    }
}
