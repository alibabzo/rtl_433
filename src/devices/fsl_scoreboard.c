/* FSL Scoreboard Remote
 */

#include "decoder.h"

static char *fsl_stat_name[] = {
    "brightness",
    "lh_no",
    "rh_no",
    "lh_score",
    "rh_score",
    "total",
    "wickets",
    "overs",
    "extras",
    "last_man",
    "last_wkt",
    "1st_inns",
    "runs_rqd",
    "ovs_left",
    "bowler_1",
    "bowler_2",
};

static inline int fsl_digit(uint8_t digit) {
    /* Digits which should not be displayed have a value of 0xf.
     * Convert them to 0.
     */
    return (digit == 0xf) ? 0 : digit;
}


static int fsl_scoreboard_callback(r_device *decoder, bitbuffer_t *bitbuffer) {
    data_t *data;

    /* Reject codes of wrong length */
    if (bitbuffer->bits_per_row[0] != 70 && bitbuffer->bits_per_row[0] != 71)
        return 0;


    bitbuffer_t databits = {0};
    unsigned pos = bitbuffer_manchester_decode(bitbuffer, 0, 3, &databits, 33);

    /* Reject codes when Manchester decoding fails */
    if (pos != 69)
        return 0;

    bitrow_t *bb = databits.bb;
    uint8_t *b = bb[0];

    uint8_t stat = b[0] & 0xf;
    uint8_t hundreds = b[1] & 0xf;
    uint8_t tens = b[2] & 0xf;
    uint8_t units = b[3] & 0xf;
    uint8_t parity = b[4] >> 7; // TODO: figure out checksum
    uint16_t score_value = fsl_digit(hundreds) * 100 + fsl_digit(tens) * 10 + fsl_digit(units);

    data = data_make("model", "", DATA_STRING, "FSL Scoreboard Remote",
                     "statistic", "Statistic", DATA_STRING, fsl_stat_name[stat],
                     "hundreds", "Hundreds digit", DATA_INT, hundreds,
                     "tens", "Tens digit", DATA_INT, tens,
                     "units", "Units digit", DATA_INT, units,
                     "value", "Value", DATA_INT, score_value,
                     NULL);

    decoder_output_data(decoder, data);

    return 0;
}

static char *output_fields[] = {
    "statistic",
    "hundreds",
    "tens",
    "units",
    "value",
    NULL
};

r_device fsl_scoreboard = {
    .name = "FSL Scoreboard Remote",
    .modulation = FSK_PULSE_PCM,
    .short_width = 550,
    .long_width = 550,
    .reset_limit = 1200,
    .decode_fn = &fsl_scoreboard_callback,
    .disabled = 0,
    .fields = output_fields,
};


