// uC_terminfo.h
// -----------------------------------------------------------------------

#ifndef UC_TERMINFO_H
#define UC_TERMINFO_H

// -----------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "uC_parse.h"

#define KEY_BUFF_SZ (32)

// -----------------------------------------------------------------------

typedef struct
{
    int8_t *ti_map;             // memory mapped address of terminfo file
    int16_t ti_size;            // size of memory mapping

    char    *ti_names;          // pointer to term names
    int8_t  *ti_bool;           // pointer to terminfo flags
    int16_t *ti_numbers;        // pointer to terminfo numbers
    int16_t *ti_strings;
    char    *ti_table;

    int8_t wide;                // numbers item size size shift factor
} __attribute__((__packed__)) ti_file_t;

// -----------------------------------------------------------------------

typedef struct
{
    int8_t digits;          // number of digits for %d (2 or 3)
    int8_t fsp;             // stack pointer for ...
    int64_t fstack[5];      // format string stack
    int64_t atoz[26];       // named format string variables
    int64_t AtoZ[26];

    // format string parsing and escape sequence compile buffer

    int64_t params[MAX_PARAM];  // format string parametesr
    uint8_t *esc_buff;          // format string compilation output buffer
    uint16_t num_esc;           // max of 64k of compiled escape seq bytes
    const uint8_t *f_str;      // pointer to next char of format string
    ti_file_t ti_file;

    bool stuffed;           // true if a key has been manually injected

    uint8_t keybuff[KEY_BUFF_SZ];
    int16_t num_k;
} ti_vars_t;

// -----------------------------------------------------------------------
// terminfo name, terminfo strings section offset to format string

// it would be ***REALLY*** freeking nice if gcc gave me a way to totally
// reset the value returned by __COUNTER__ so I can guarantee it starts
// at zero here.

// the value passed to the uC_format() function here is the terminfo
// strings section offset for the format string used to compile the
// specific escape sequences for each operation

#define ti_bt()    uC_format(0)
#define ti_bell()  uC_format(1)   // NOOOOO!
#define ti_cr()    uC_format(2)
#define ti_csr()   uC_format(3)   // change scroll region
#define ti_tbc()   uC_format(4)   // clear all tab stops
#define ti_clear() uC_format(5)   // clear screen
#define ti_el()    uC_format(6)   // erase line
#define ti_ed()    uC_format(7)   // clear to end of screen
#define ti_hpa()   uC_format(8)   // set cursor horizontal position
#define ti_cmdch() uC_format(9)   // terminal settable cmd character in prototype !?
#define ti_cup()   uC_format(10)  // set cursor horizontal / vertical
#define ti_cud1()  uC_format(11)  // move cursor down 1 line
#define ti_home()  uC_format(12)  // move cursor to home location
#define ti_civis() uC_format(13)  // make cursor invisible
#define ti_cub1()  uC_format(14)  // move cursor back one on line
#define ti_mrcup() uC_format(15)  // memory relative cursor addressing
#define ti_cnorm() uC_format(16)  // make cursor visible
#define ti_cuf1()  uC_format(17)  // move cursor forward on line
#define ti_ll()    uC_format(18)  // move cursor to lower left
#define ti_cuu1()  uC_format(19)  // move cursor up one line
#define ti_cvvis() uC_format(20)  // make cursor very visible
#define ti_dch1()  uC_format(21)  // delete character
#define ti_dl1()   uC_format(22)  // delete line
#define ti_dsl()   uC_format(23)  // disable status line
#define ti_hd()    uC_format(24)  // half line down
#define ti_smacs   uC_format(25)  // start alternate char set
#define ti_blink   uC_format(26)  // turn blink on (just no!)
#define ti_bold()  uC_format(27)  // turn on bold mode
#define ti_smcup() uC_format(28)  // string to start programs using cup ?
#define ti_smdc()  uC_format(29)  // enter delete mode
#define ti_dim()   uC_format(30)  // turn on half bright mode
#define ti_smir()  uC_format(31)  // set mode insert
#define ti_invis() uC_format(32)  // turn on blank mode
#define ti_prot()  uC_format(33)  // turn on protected mode
#define ti_rev()   uC_format(34)  // turn on reverse video
#define ti_smso()  uC_format(35)  // turn on standout mode
#define ti_smul()  uC_format(36)  // turn on underline
#define ti_ech()   uC_format(37)  // erase chars
#define ti_rmacs() uC_format(38)  // end alt charset
#define ti_sgr0()  uC_format(39)  // blow away all attribs
#define ti_rmcup() uC_format(40)  // string to end programs using cup
#define ti_rmdc()  uC_format(41)  // end delete mode
#define ti_rmir()  uC_format(42)  // exit insert mode
#define ti_rmso()  uC_format(43)  // exit standout mode
#define ti_rmul()  uC_format(44)  // disable underline mode
#define ti_flash   uC_format(45)  // visible bell
#define ti_ff      uC_format(44)  // hardcopy terminal page eject
#define ti_fsl     uC_format(47)  // return from status line
#define ti_is1()   uC_format(48)  // initialization string
#define ti_is2()   uC_format(49)  // initialization string
#define ti_is3()   uC_format(50)  // initialization string
#define ti_if()    uC_format(51)  // name of initialization file
#define ti_ich1()  uC_format(52)  // insert character
#define ti_il1()   uC_format(53)  // insert line
#define ti_ip()    uC_format(54)  // insert padding after inserted character
#define ti_kbs()   uC_format(55)  // backspace key
#define ti_ktbc()  uC_format(56)  // clear-all-tabs key
#define ti_kclr()  uC_format(57)  // clear-screen or erase key
#define ti_kctab() uC_format(58)  // clear-tab key
#define ti_kdch1() uC_format(59)  // delete-character key
#define ti_kdl1()  uC_format(60)  // delete-line key
#define ti_kcud1() uC_format(61)  // down-arrow key
#define ti_krmir() uC_format(62)  // sent by rmir or smir in insert mode
#define ti_kel()   uC_format(63)  // clear-to-end-of-line key
#define ti_ked()   uC_format(64)  // clear-to-end-of-screen key
#define ti_kf0()   uC_format(65)  // F0 function key ????
#define ti_kf1()   uC_format(66)  // F1 function key
#define ti_kf10()  uC_format(67)  // F10 function key
#define ti_kf2()   uC_format(68)  // F2 function key
#define ti_kf3()   uC_format(69)  // F3 function key
#define ti_kf4()   uC_format(70)  // F4 function key
#define ti_kf5()   uC_format(71)  // F5 function key
#define ti_kf6()   uC_format(72)  // F6 function key
#define ti_kf7()   uC_format(73)  // F7 function key
#define ti_kf8()   uC_format(74)  // F8 function key
#define ti_kf9()   uC_format(75)  // F9 function key
#define ti_khome() uC_format(76)  // home key
#define ti_kich1() uC_format(77)  // insert-character key
#define ti_kil1()  uC_format(78)  // insert-line key
#define ti_kcub1() uC_format(79)  // left-arrow key
#define ti_kll()   uC_format(80)  // lower-left key (home down)
#define ti_knp()   uC_format(81)  // next-page key
#define ti_kpp()   uC_format(82)  // previous-page key
#define ti_kcuf1() uC_format(83)  // right-arrow key
#define ti_kind()  uC_format(84)  // scroll-forward key
#define ti_kri()   uC_format(85)  // scroll-backward key
#define ti_khts()  uC_format(86)  // set-tab key
#define ti_kcuu1() uC_format(87)  // up-arrow key
#define ti_rmkx()  uC_format(88)  // leave 'keyboard_transmit' mode
#define ti_smkx()  uC_format(89)  // enter 'keyboard_transmit' mode
#define ti_lf0()   uC_format(90)  // label on function key f0 if not f0
#define ti_lf1()   uC_format(91)  // label on function key f1 if not f1
#define ti_lf10()  uC_format(92)  // label on function key f10 if not f10
#define ti_lf2()   uC_format(93)  // label on function key f2 if not f2
#define ti_lf3()   uC_format(94)  // label on function key f3 if not f3
#define ti_lf4()   uC_format(95)  // label on function key f4 if not f4
#define ti_lf5()   uC_format(96)  // label on function key f5 if not f5
#define ti_lf6     uC_format(97)  // label on function key f6 if not f6
#define ti_lf7     uC_format(98)  // label on function key f7 if not f7
#define ti_lf8     uC_format(99)  // label on function key f8 if not f8
#define ti_lf9     uC_format(100) // label on function key f9 if not f9
#define ti_rmm     uC_format(101) // turn off meta mode
#define ti_smm     uC_format(102) // turn on meta mode (8th-bit on)
#define ti_nel     uC_format(103) // newline (behave like cr followed by lf)
#define ti_pad     uC_format(104) // padding char (instead of null)
#define ti_dch()   uC_format(105) // delete n characters
#define ti_dl()    uC_format(106) // delete n lines
#define ti_cud()   uC_format(107)  // move cursor down n lines
#define ti_ich()   uC_format(108)  // insert n characters
#define ti_indn()  uC_format(109)  // scroll forward n lines
#define ti_il()    uC_format(110)  // insert n lines
#define ti_cub()   uC_format(111)  // move cursor back n chars
#define ti_cuf()   uC_format(112)  // move cursor forward
#define ti_rin()   uC_format(113)  // scroll back lines
#define ti_cuu()   uC_format(114)  // cursor up lines
#define ti_pfkey() uC_format(115)  // program f key to type string
#define ti_pfloc() uC_format(116)  // program f key to execute string
#define ti_pfx()   uC_format(117)  // program f key to tx string
#define ti_mc0()   uC_format(118)  // print screen
#define ti_mc4()   uC_format(119)  // turn off printer
#define ti_mc5()   uC_format(120)  // turn on printer
#define ti_rep()   uC_format(121)  // repeat char
#define ti_rs1()   uC_format(122)  // reset string
#define ti_rs2()   uC_format(123)  // reset string
#define ti_rs3()   uC_format(124)  // reset string
#define ti_rf()    uC_format(125)  // name of reset file
#define ti_rc()    uC_format(126)  // restore cursor location
#define ti_vpa()   uC_format(127)  // vertical position absolute
#define ti_sc()    uC_format(128)  // save cursor location
#define ti_ind()   uC_format(129)  // scroll up
#define ti_ri()    uC_format(130)  // scroll down
#define ti_sgr()   uC_format(131)  // set graphics rendering
#define ti_hts()   uC_format(132)  // tab to next 8 space stop
#define ti_wind()  uC_format(133)  // set current window dims
#define ti_ht()    uC_format(134)  // tab to next stop
#define ti_tsl()   uC_format(135)  // move to status line
#define ti_uc()    uC_format(136)  // underline char
#define ti_hu()    uC_format(137)  // half line up
#define ti_iprog() uC_format(138)  // path of init prog
#define ti_ka1()   uC_format(139)  // keypad upper left
#define ti_ka3()   uC_format(140)  // keypad upper right
#define ti_kb2()   uC_format(141)  // kepad center
#define ti_kc1()   uC_format(142)  // keypad lower left
#define ti_kc3()   uC_format(143)  // keypad lower right
#define ti_mc5p()  uC_format(144)  // turn on printer for n bytes
#define ti_rmp()   uC_format(145)  // like ti_ip when in insert mode
#define ti_acsc()  uC_format(146)  // alt charset pairs
#define ti_pln()   uC_format(147)  // program label to show string
#define ti_kcbt()  uC_format(148)  // backtab key
#define ti_smxon() uC_format(149)  // turn on xon/xoff
#define ti_rmxon() uC_format(150)  // turn off xon/xoff
#define ti_smam()  uC_format(151)  // turn on auto margins
#define ti_rmam()  uC_format(152)  // turn off auto margins
#define ti_xonc()  uC_format(153)  // xon char
#define ti_xoffc() uC_format(154)  // xoff char
#define ti_enacs() uC_format(155)  // enable alt charset
#define ti_smln()  uC_format(156)  // turn on soft labels
#define ti_rmln()  uC_format(157)  // turn off soft labels
#define ti_kbeg()  uC_format(158)  // begin key
#define ti_kcan()  uC_format(159)  // cancel key
#define ti_kclo()  uC_format(160)  // close key
#define ti_cmd()   uC_format(161)  // command key
#define ti_cpy()   uC_format(162)  // copy key
#define ti_kcrt()  uC_format(163)  // create key (the God key!)
#define ti_kend()  uC_format(164)  // key end
#define ti_kent()  uC_format(165)  // key enter
#define ti_kext()  uC_format(166)  // exit key
#define ti_kfnd()  uC_format(167)  // find key
#define ti_khlp()  uC_format(168)  // help key
#define ti_kmrk()  uC_format(169)  // mark key (my key!)
#define ti_kmsg()  uC_format(170)  // message key
#define ti_kmov()  uC_format(171)  // move key
#define ti_knxt()  uC_format(172)  // next key
#define ti_kopn()  uC_format(173)  // open key
#define ti_kopt()  uC_format(174)  // options key
#define ti_kprv()  uC_format(175)  // previous key
#define ti_kprt()  uC_format(176)  // print key
#define ti_krdo()  uC_format(177)  // redo key
#define ti_kref()  uC_format(178)  // reference key
#define ti_krfr()  uC_format(179)  // refresh key
#define ti_krpl()  uC_format(180)  // replace key
#define ti_krst()  uC_format(181)  // restart key
#define ti_kres()  uC_format(182)  // resume key
#define ti_ksav()  uC_format(183)  // save key
#define ti_kspd()  uC_format(184)  // suspend key
#define ti_kund()  uC_format(185)  // undo key
#define ti_kBEG()  uC_format(186)  // shift begin key
#define ti_kCAN()  uC_format(187)  // shift cancel key
#define ti_kCMD()  uC_format(188)  // shift command key
#define ti_kCPY()  uC_format(189)  // shift copy key
#define ti_kCRT()  uC_format(190)  // shift create key
#define ti_kDC()   uC_format(191)  // shift delete character key
#define ti_kDL()   uC_format(192)  // shift delete line key
#define ti_kslt()  uC_format(193)  // select key
#define ti_kEND()  uC_format(194)  // shifted end key
#define ti_kEOL()  uC_format(195)  // shift clear to end of line key
#define ti_kEXT()  uC_format(196)  // shift exit key
#define ti_kFND()  uC_format(197)  // shift find key
#define ti_kHLP()  uC_format(198)  // shift help key
#define ti_kHOM()  uC_format(199)  // shift home key
#define ti_kIC()   uC_format(200)  // shift insert character key
#define ti_kLFT()  uC_format(201)  // shift cursor left
#define ti_kMSG()  uC_format(202)  // shift message key
#define ti_kMOV()  uC_format(203)  // shift move key
#define ti_kNXT()  uC_format(204)  // shift next key
#define ti_kOPT()  uC_format(205)  // shift options key
#define ti_kPRV()  uC_format(206)  // shift previous key
#define ti_kPRT()  uC_format(207)  // shift print key
#define ti_kRDO()  uC_format(208)  // shift redo key
#define ti_kRPL()  uC_format(209)  // shift replace key
#define ti_kRIT()  uC_format(210)  // shift cursor right
#define ti_kRES()  uC_format(211)  // shift resume key
#define ti_kSAV()  uC_format(212)  // shift save key
#define ti_kSPD()  uC_format(213)  // shift suspend key
#define ti_kUND()  uC_format(214)  // shift undo key
#define ti_rfi()   uC_format(215)  // send next input char (for ptys)
#define ti_kf11()  uC_format(216)  //
#define ti_kf12()  uC_format(217)  //

#define ti_kf13()  uC_format(218)  // Shift + function keys
#define ti_kf14()  uC_format(219)  //
#define ti_kf15()  uC_format(220)  //
#define ti_kf16()  uC_format(221)  //
#define ti_kf17()  uC_format(222)  //
#define ti_kf18()  uC_format(223)  //
#define ti_kf19()  uC_format(224)  //
#define ti_kf20()  uC_format(225)  //
#define ti_kf21()  uC_format(226)  //
#define ti_kf22()  uC_format(227)  //
#define ti_kf23()  uC_format(228)  //
#define ti_kf24()  uC_format(229)  //

#define ti_kf25()  uC_format(230)  // Control + function keys
#define ti_kf26()  uC_format(231)  //
#define ti_kf27()  uC_format(232)  //
#define ti_kf28()  uC_format(233)  //
#define ti_kf29()  uC_format(234)  //
#define ti_kf30()  uC_format(235)  //
#define ti_kf31()  uC_format(236)  //
#define ti_kf32()  uC_format(237)  //
#define ti_kf33()  uC_format(238)  //
#define ti_kf34()  uC_format(239)  //
#define ti_kf35()  uC_format(240)  //
#define ti_kf36()  uC_format(241)  //

#define ti_kf37()  uC_format(242)  // shfift + control + function keys
#define ti_kf38()  uC_format(243)  //
#define ti_kf39()  uC_format(244)  //
#define ti_kf40()  uC_format(245)  //
#define ti_kf41()  uC_format(246)  //
#define ti_kf42()  uC_format(247)  //
#define ti_kf43()  uC_format(248)  //
#define ti_kf44()  uC_format(249)  //
#define ti_kf45()  uC_format(250)  //
#define ti_kf46()  uC_format(251)  //
#define ti_kf47()  uC_format(252)  //
#define ti_kf48()  uC_format(253)  //

#define ti_kf49()  uC_format(254)  // shift + alt + control + f key?
#define ti_kf50()  uC_format(255)  //
#define ti_kf51()  uC_format(256)  //
#define ti_kf52()  uC_format(257)  //
#define ti_kf53()  uC_format(258)  //
#define ti_kf54()  uC_format(259)  //
#define ti_kf55()  uC_format(260)  //
#define ti_kf56()  uC_format(261)  //
#define ti_kf57()  uC_format(262)  //
#define ti_kf58()  uC_format(263)  //
#define ti_kf59()  uC_format(264)  //
#define ti_kf60()  uC_format(265)  //

#define ti_kf61()  uC_format(266)  //  ???
#define ti_kf62()  uC_format(267)  //  ???
#define ti_kf63()  uC_format(268)  //  ???

#define ti_el1()   uC_format(269)  // clear to beginning of line
#define ti_mgc()   uC_format(270)  // clear left / right soft margins
#define ti_smgl()  uC_format(271)  // set left soft margin
#define ti_smgr()  uC_format(272)  // set right soft margin
#define ti_fln()   uC_format(273)  // label format
#define ti_sclk()  uC_format(274)  // set clock h/m/s
#define ti_dclk()  uC_format(275)  // display clock
#define ti_rmclk() uC_format(276)  // remove clock
#define ti_win()   uC_format(277)  // define window
#define ti_wingi() uC_format(278)  // go to window n
#define ti_hup()   uC_format(279)  // hang up phone
#define ti_dial()  uC_format(280)  // dial number n
#define ti_qdial() uC_format(281)  // dial number n without checking
#define ti_tone()  uC_format(282)  // select touch tone dialing
#define ti_pulse() uC_format(283)  // select pulse dialing
#define ti_hook()  uC_format(284)  // flash hook (ye olde black+white moivies!)
#define ti_pause() uC_format(285)  // pause for 2-3 secs
#define ti_wait()  uC_format(286)  // wait for dial tone
#define ti_u0()    uC_format(287)  // user strings
#define ti_u1()    uC_format(288)  // user strings
#define ti_u2()    uC_format(289)  // user strings
#define ti_u3()    uC_format(290)  // user strings
#define ti_u4()    uC_format(291)  // user strings
#define ti_u5()    uC_format(292)  // user strings
#define ti_u6()    uC_format(293)  // user strings
#define ti_u7()    uC_format(294)  // user strings
#define ti_u8()    uC_format(295)  // user strings
#define ti_u9()    uC_format(296)  // user strings
#define ti_op()    uC_format(297)  // set default color pair to original
#define ti_oc()    uC_format(298)  // set all coror pairs to original
#define ti_initc() uC_format(299)  // initialize color n
#define ti_initp() uC_format(300)  // initialize color pair
#define ti_scp()   uC_format(301)  // set current color pair
#define ti_setf()  uC_format(302)  // set fg color
#define ti_setb()  uC_format(303)  // set bg color
#define ti_cpi()   uC_format(304)  // change chars per inch
#define ti_lpi()   uC_format(305)  // change lines per inch
#define ti_chr()   uC_format(306)  // change horizontal resolution
#define ti_cvr()   uC_format(307)  // change vertical resolution
#define ti_defc()  uC_format(308)  // define char
#define ti_swidm() uC_format(309)  // enter dobuel wide mode
#define ti_sdrfq() uC_format(310)  // enter draft quality mode
#define ti_sitm()  uC_format(311)  // enter italics mode
#define ti_slm()   uC_format(312)  // start left carraige motion
#define ti_smicm() uC_format(313)  // start micro motion mode
#define ti_snlq()  uC_format(314)  // enter LNQ mode
#define ti_snrmq() uC_format(315)  // enter normal quality mode
#define ti_sshm()  uC_format(316)  // enter shadow print mode
#define ti_ssubm() uC_format(317)  // enter subscript mode
#define ti_ssupm() uC_format(318)  // enter superscript mode
#define ti_sum()   uC_format(319)  // start upward carraige motion
#define ti_rwidm() uC_format(320)  // end double wide mode
#define ti_ritm()  uC_format(321)  // end italics mode
#define ti_rlm()   uC_format(322)  // end left motion mode
#define ti_rmicm() uC_format(323)  // end micro motion mode
#define ti_rshm()  uC_format(324)  // end shadow print mode
#define ti_rsubm() uC_format(325)  // end subscript mode
#define ti_rsupm() uC_format(326)  // end superscript mode
#define ti_rum()   uC_format(327)  // end reverse character mode
#define ti_mhpa()  uC_format(328)  // like hpa but in micro mode
#define ti_mcud1() uC_format(329)  // like cud1 but in micro mode
#define ti_mcub1() uC_format(330)  // like cub1 but in micro mode
#define ti_mcuf1() uC_format(331)  // like cuf1 but in micro mode
#define ti_mvpa()  uC_format(332)  // like vpa but in micro mode
#define ti_mcuu1() uC_format(333)  // like cuu1 but in micro mode
// why did this one have to be one char longer? grrr!
// it messes with my OCD!!!!
#define ti_porder() uC_format(334) // match sw bits to print head pins
#define ti_mcud()  uC_format(335)  // like cud but in micro mode
#define ti_mcub()  uC_format(336)  // like cub but in micro mode
#define ti_mcuf()  uC_format(337)  // like cuf but in micro mode
#define ti_mcuu()  uC_format(338)  // like cuu but in micro mode
#define ti_scs()   uC_format(339)  // select char set n
#define ti_smbg()  uC_format(340)  // set bottom margin
#define ti_smbgp() uC_format(341)  // set bottom margin to n
#define ti_smglp() uC_format(342)  // set left margin to n
#define ti_smgrp() uC_format(343)  // set right margin to n
#define ti_smgt()  uC_format(344)  // set top margin
#define ti_smgtp() uC_format(345)  // set bottom margin to n
#define ti_sbim()  uC_format(346)  // start printing bit image gfx
#define ti_scsd()  uC_format(347)  // start char set def
#define ti_rbim()  uC_format(348)  // stop printing bit image gfx
#define ti_rcsd()  uC_format(349)  // end charset definition
#define ti_subcs() uC_format(350)  // list of subscriptable chars
#define ti_supcs() uC_format(351)  // list of superscriptable chars
#define ti_docr()  uC_format(352)  // list of chars that do cr when printed
#define ti_zerom() uC_format(353)  // no motion for next char
#define ti_csnm()  uC_format(354)  // produce item from list of set names

#define ti_kmous() uC_format(355)  // mouse envent has occurred
#define ti_minfo() uC_format(356)  // mouse info
#define ti_reqmp() uC_format(357)  // request mouse position
#define ti_getm()  uC_format(358)  // get button events
#define ti_setaf() uC_format(359)  // set fg using ansi
#define ti_setab() uC_format(360)  // set bg using ansi

// -----------------------------------------------------------------------

API void uC_curoff(void);
API void uC_curon(void);
API void uC_clear(void);
API void uC_hpa(uint16_t x);
API void uC_cup(uint16_t x, uint16_t y);
API void uC_cud1(void);
API void uC_home(void);
API void uC_cub1(void);
API void uC_cuf1(void);
API void uC_cuu1(void);
API void uC_dch1(void);
API void uC_cud(uint16_t n1);
API void uC_ich(void);
API void uC_cub(uint16_t n1);
API void uC_cuf(uint16_t n1);
API void uC_cuu(uint16_t n1);
API void uC_vpa(uint16_t y1);
API void uC_cr(void);
API void uC_smkx(void);
API void uC_rmkx(void);

// -----------------------------------------------------------------------

#endif // UC_TERMINFO_H

// =======================================================================
