#include <assert.h>
#include <unistd.h>
#include "uart_bidir_protocol.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>

void print_msp_packet(msp_packet* packet) {
    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
    
    fprintf(stderr, "MSP packet source file: %hhx\n", packet->src_file);
    fprintf(stderr, "MSP packet source rank: %hhx\n", packet->src_rank);
    fprintf(stderr, "MSP packet dest file: %hhx\n", packet->dest_file);
    fprintf(stderr, "MSP packet dest rank: %hhx\n", packet->dest_rank);
    fprintf(stderr, "MSP packet ptype: %hhx\n", packet->ptype);
    fprintf(stderr, "MSP packet button event: %hhx\n", packet->button_event);

    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
}

void print_rpi_move(rpi_move* move) { 
    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

    fprintf(stderr, "RPI move source file: %hhx\n", move->src_file);
    fprintf(stderr, "RPI move source rank: %hhx\n", move->src_rank);
    fprintf(stderr, "RPI move dest file: %hhx\n", move->dest_file);
    fprintf(stderr, "RPI move dest rank: %hhx\n", move->dest_rank);

    fprintf(stderr, "RPI move ptype: %hhx\n", move->ptype);

    fprintf(stderr, "RPI move M2 bit: %hhx\n", move->m2);

    fprintf(stderr, "RPI move M2 source file: %hhx\n", move->m2_src_file);
    fprintf(stderr, "RPI move M2 source rank: %hhx\n", move->m2_src_rank);
    fprintf(stderr, "RPI move M2 dest file: %hhx\n", move->m2_dest_file);
    fprintf(stderr, "RPI move M2 dest rank: %hhx\n", move->m2_dest_rank);

    fprintf(stderr, "RPI move M2 mtype: %hhx\n", move->mtype);
    fprintf(stderr, "RPI move M2 ptype: %hhx\n", move->m2_ptype);
    fprintf(stderr, "RPI move last bit: %hhx\n", move->last);

    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
}

void print_rpi_undo(rpi_undo* undo) {
    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

    fprintf(stderr, "RPI undo source file: %hhx\n", undo->src_file);
    fprintf(stderr, "RPI undo source rank: %hhx\n", undo->src_rank);
    fprintf(stderr, "RPI undo dest file: %hhx\n", undo->dest_file);
    fprintf(stderr, "RPI undo dest rank: %hhx\n", undo->dest_rank);

    fprintf(stderr, "RPI undo ptype: %hhx\n", undo->ptype);

    fprintf(stderr, "RPI undo M2 bit: %hhx\n", undo->m2);

    fprintf(stderr, "RPI undo M2 source file: %hhx\n", undo->m2_src_file);
    fprintf(stderr, "RPI undo M2 source rank: %hhx\n", undo->m2_src_rank);
    fprintf(stderr, "RPI undo M2 dest file: %hhx\n", undo->m2_dest_file);
    fprintf(stderr, "RPI undo M2 dest rank: %hhx\n", undo->m2_dest_rank);

    fprintf(stderr, "RPI undo B/W flag: %hhx\n", undo->bw_flag);
    fprintf(stderr, "RPI undo undone ptype: %hhx\n", undo->undone_ptype);

    for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
}

int main() {

    // Test 1: Check that MSP packet encoding and decoding recovers original word "on the line"
    msp_packet tested_msp_packet = {0};
    uint32_t simple_msp_moveword = 0x35660000;
    vdecode_packet_for_rpi(simple_msp_moveword, &tested_msp_packet);

    print_msp_packet(&tested_msp_packet);
    assert(xencode_move_for_rpi(&tested_msp_packet) == simple_msp_moveword);

    // Test 2: check that RPI move packet encoding and decoding recovers original word "on the line"
    rpi_move tested_move = {0};
    uint32_t simple_rpi_moveword = 0x925c0000;

    vdecode_move_for_msp(simple_rpi_moveword, &tested_move);
    print_rpi_move(&tested_move);
    assert(xencode_move_for_msp(&tested_move) == simple_rpi_moveword);

    // Test 3: check that invalid move encodings (e.g., nonzero lower 16 bits for m2 == 0) do not influence encoding and decoding.
    // In other words, check "garbage in, garbage out" behavior
    uint32_t botched_simple_rpi_moveword = 0x925cffff;

    vdecode_move_for_msp(botched_simple_rpi_moveword, &tested_move);
    print_rpi_move(&tested_move);
    assert(xencode_move_for_msp(&tested_move) == botched_simple_rpi_moveword);

    // Test 4: check encoding and decoding behavior for "complex" RPI move packet with meaningful lower 16 bits
    uint32_t complex_rpi_moveword = 0x106f1c5f;

    vdecode_move_for_msp(complex_rpi_moveword, &tested_move);
    print_rpi_move(&tested_move);
    assert(xencode_move_for_msp(&tested_move) == complex_rpi_moveword);

    // Test 5: check encoding and decoding behavior for RPI undo packet (simple B)
    rpi_undo tested_undo = {0}; 
    uint32_t simple_undo_b = 0x10320008;

    vdecode_undo_for_msp(simple_undo_b, &tested_undo);
    print_rpi_undo(&tested_undo);
    assert(xencode_undo_for_msp(&tested_undo) == simple_undo_b);

    // Test 6: check encoding and decoding behavior for RPI undo packet (simple W)
    uint32_t simple_undo_w = 0x10320000;
    vdecode_undo_for_msp(simple_undo_w, &tested_undo);
    print_rpi_undo(&tested_undo);
    assert(xencode_undo_for_msp(&tested_undo) == simple_undo_w);

    // Test 7: check encoding and decoding for undo capture (complex B)
    uint32_t undo_capture_b = 0x54c3015b;
    vdecode_undo_for_msp(undo_capture_b, &tested_undo);
    print_rpi_undo(&tested_undo);
    assert(xencode_undo_for_msp(&tested_undo) == undo_capture_b);

    // Test 8: check encoding and decoding for undo capture (complex W)
    uint32_t undo_capture_w = 0x54c30153;
    vdecode_undo_for_msp(undo_capture_w, &tested_undo);
    print_rpi_undo(&tested_undo);
    assert(xencode_undo_for_msp(&tested_undo) == undo_capture_w);

    /**
     * Begin "in-depth" fork tests
     */

    int pi_via_msp_fds[2];
    
    if (pipe(pi_via_msp_fds)) {
        fprintf(stderr, "ERROR: failed to create pipe from MSP to PI (%s)\n", strerror(errno));
    }

    int msp_via_pi_fds[2];

    if (pipe(msp_via_pi_fds)) {
        fprintf(stderr, "ERROR: failed to create pipe from PI to MSP (%s)\n", strerror(errno));
    }

    pid_t rpi_pid = fork();

    if (rpi_pid == 0) {
        /* Child (Raspberry Pi) */
        close(pi_via_msp_fds[1]); // Only need to read from index 0, so close write-end
        int pi_read_end = pi_via_msp_fds[0];

        close(msp_via_pi_fds[0]); // Only need to write to index 1, so close read-end
        int pi_write_end = msp_via_pi_fds[1];

        /* BEGIN next testing */
        ssize_t received = 0;

        /* Test 9c: read start packet from MSP */
        uint32_t recvd_start_packet = xrecv_packet_common(pi_read_end, &received);

        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "PI: received 0x%08x (%ld byte(s)).\n", recvd_start_packet, received);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        /* Test 10c: read restart packet from MSP */
        received = 0;
        uint32_t recvd_restart_packet = xrecv_packet_common(pi_read_end, &received);

        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "PI: received 0x%08x (%ld byte(s)).\n", recvd_restart_packet, received);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        // Epilogue code: close remaining resources and exit
        close(pi_read_end);
        close(pi_write_end);
        exit(0);
    } else {
        /* Parent (MSPM0) */
        close(msp_via_pi_fds[1]); // Only need to read from index 0, so close write-end
        int msp_read_end = msp_via_pi_fds[0];

        close(pi_via_msp_fds[0]); // Only need to write to index 1, so close read-end
        int msp_write_end = pi_via_msp_fds[1];

        /* BEGIN next testing */
        ssize_t sent = 0;

        /* Case 9p: write start packet to RPI */
        uint32_t tested_start_packet = xencode_start_for_rpi();

        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "MSP: sending 0x%08x.\n", tested_start_packet);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        sent = xsend_packet_common(tested_start_packet, msp_write_end);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "MSP: wrote %ld byte(s).\n", sent);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        /* Case 10p: write restart packet to RPI */
        uint32_t tested_restart_packet = xencode_restart_for_rpi();

        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "MSP: sending 0x%08x.\n", tested_restart_packet);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        sent = xsend_packet_common(tested_restart_packet, msp_write_end);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");
        fprintf(stderr, "MSP: wrote %ld byte(s).\n", sent);
        for (int i = 0; i < 20; i += 1) { fprintf(stderr, "="); } fprintf(stderr, "\n");

        // Epilogue code: close remaining resources and wait on child
        close(msp_read_end);
        close(msp_write_end);
        waitpid(-1, NULL, 0);
    }

    return 0;
}
