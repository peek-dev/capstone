#ifndef CAPSTONE_CLOCK_PRIVATE_H
#define CAPSTONE_CLOCK_PRIVATE_H
enum Clock_MsgType {
    clockmsg_set_time,
    clockmsg_set_increment,
    clockmsg_render_state,
    clockmsg_set_state,
    clockmsg_set_turn,
    clockmsg_test_segments,
    clockmsg_set_numbers
};

typedef struct {
    enum Clock_MsgType type;
    union {
        uint32_t times[2];
        uint8_t seconds_per_test;
        game_turn turn;
        clock_state state;
        uint16_t numbers[2];
    };
} Clock_Message;
#endif