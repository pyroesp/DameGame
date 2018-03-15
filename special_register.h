#ifndef _SPECIAL_REGISTER_H
#define _SPECIAL_REGISTER_H

union Special_Register{
	uint8_t *reg[0xFF]; /* point to FF00 in the memory */
	struct{
		union{
			uint8_t P1; /* Reading joypad */
			union{
				uint8_t P10 : 1;
				uint8_t P11 : 1;
				uint8_t P12 : 1;
				uint8_t P13 : 1;
				uint8_t P14 : 1;
				uint8_t P15 : 1;
				uint8_t b7_unused : 1;
				uint8_t b6_unused : 1;
			}P1_bits;
		};
		uint8_t SB; /* Serial Transfer Data */
		union{
			uint8_t SC; /* SIO Control */
			union{
				uint8_t clock : 1;
				uint8_t unused : 6;
				uint8_t transfer_flag : 1;
			}SC_bits;
		};
		uint8_t DIV; /* Divider Register */
		uint8_t TIMA; /* Timer Counter */
		uint8_t TMA; /* Timer Modulo */
		union{
			uint8_t TAC;
			union{
				uint8_t clock_select : 2;
				uint8_t timer_stop : 1;
				uint8_t unused : 5;
			}TAC_bits;
		};
		union{
			uint8_t IF; /* Interrupt flag */
			union{
				uint8_t v_blank : 1;
				uint8_t lcdc : 1;
				uint8_t timer_overflow : 1;
				uint8_t serial_transfer_complete : 1;
				uint8_t falling_edge_P1 : 1;
				uint8_t unused : 3;
			}IF_bits;
		};
		union{
			uint8_t NR_10;
			union{
				uint8_t sweep_shift : 3;
				uint8_t sweep_inc_dec : 1;
				uint8_t sweep_time : 3;
				uint8_t unused : 1;
			}NR_10_bits;
		};
		union{
			uint8_t NR_11;
			union{
				uint8_t sound_len : 6;
				uint8_t wave_patern_duty : 2;
			}NR_11_bits;
		};
		union{
			uint8_t NR_12;
			union{
				uint8_t envelope_sweep : 3;
				uint8_t envelope : 1;
				uint8_t envelope_volume : 4;
			}NR_12_bits;
		};
		union{
		    struct{
                union{
                    uint8_t NR_13;
                    uint8_t freq_lo_0;
                };
                union{
                    uint8_t NR_14;
                    struct{
                        uint8_t freq_hi : 3;
                        uint8_t unused : 3;
                        uint8_t sound_mode : 1; // consecutive or counter mode
                    }NR_14_bits;
                };
		    };
			struct{
				uint16_t freq : 11;
				uint8_t unused : 5;
			}sound_0;
		};
		union{
			uint8_t NR_21;
			struct{
				uint8_t sound_len : 6;
				uint8_t wave_pattern_duty : 2;
			}NR_21_bits;
		};
		union{
			uint8_t NR_22;
			struct{
				uint8_t envelope_sweep : 3;
				uint8_t envelope : 1;
				uint8_t envelope_volume : 4;
			}NR_22_bits;
		};
		union{
		    struct{
                union{
                    uint8_t NR_23;
                    uint8_t freq_lo_1;
                };
                union{
                    uint8_t NR_24;
                    struct{
                        uint8_t freq_hi : 3;
                        uint8_t unused : 3;
                        uint8_t sound_mode : 1; // consecutive or counter mode
                    }NR_24_bits;
                };
		    };
		    uint16_t sound_1;
			struct{
				uint16_t freq : 11;
				uint8_t unused : 5;
			}sound_1_bits;
		};
		union{
			uint8_t NR_30;
			struct{
				uint8_t unused : 7;
				uint8_t sound_output : 1; // off / on
			}NR_30_bits;
		};
		uint8_t NR_31; // sound length
		union{
			uint8_t NR_32;
			struct{
				uint8_t unused0 : 5;
				uint8_t select_output : 2;
				uint8_t unused1 : 1;
			}NR_32_bits;
		};
		uint8_t NR_33;
		uint8_t NR_34;

		uint8_t NR_41;
		uint8_t NR_42;
		uint8_t NR_43;
		uint8_t NR_44;

		uint8_t NR_50;
		uint8_t NR_51;
		uint8_t NR_52;

		uint8_t unused0[0x3];

		uint8_t wave_pattern[0xF];

		uint8_t LCDC;
		uint8_t STAT; // LCDC status
		uint8_t SCY;
		uint8_t SCX;
		uint8_t LY;
		uint8_t LYC;
		uint8_t DMA;
		uint8_t BGP;
		uint8_t OBP0;
		uint8_t OBP1;
		uint8_t WY;
		uint8_t WX;
		uint8_t unused1[0xB4];
		uint8_t IE;
	};
};

#endif
