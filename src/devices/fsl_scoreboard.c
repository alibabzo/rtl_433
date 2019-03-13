/* FSL Scoreboard Remote
 */

#include "decoder.h"

char *fsl_stat_name(uint8_t command) {
    char *out = "0xxx";
    switch(command) {
        case 0x0: return "brightness"; break;
        case 0x1: return "lh_no";      break;
        case 0x2: return "rh_no";      break;
        case 0x3: return "lh_score";   break;
        case 0x4: return "rh_score";   break;
        case 0x5: return "total";      break;
        case 0x6: return "wickets";    break;
        case 0x7: return "overs";      break;
        case 0x8: return "extras";     break;
        case 0x9: return "last_man";   break;
        case 0xa: return "last_wkt";   break;
        case 0xb: return "1st_inns";   break;
        case 0xc: return "runs_rqd";   break;
        case 0xd: return "ovs_left";   break;
        case 0xe: return "bowler_1";   break;
        case 0xf: return "bowler_2";   break;
    }
    sprintf(out, "0x%x", command);
    return out;
}

inline int fsl_digit(uint8_t digit) {
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
    uint8_t parity = b[4] >> 7;
    uint16_t score_value = fsl_digit(hundreds) * 100 + fsl_digit(tens) * 10 + fsl_digit(units);

    data = data_make("model", "", DATA_STRING, "FSL Scoreboard Remote",
                     "statistic", "Statistic", DATA_STRING, fsl_stat_name(stat),
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


