
// Marcel Timm, RhinoDevel, 2024feb09

#ifndef MT_SEND_CBS_RESULT
#define MT_SEND_CBS_RESULT

enum send_cbs_result
{
    send_cbs_result_invalid = -1,
    send_cbs_result_fast_mode_detected = 0,
    send_cbs_result_stopped = 1,
    send_cbs_result_finished = 2
};

#endif //MT_SEND_CBS_RESULT
