data 'TMPL' (128, "LPic") {
	$"1344 6566 6175 6C74 204C 616E 6775 6167"            /* .Default Languag */
	$"6520 4944 4457 5244 0543 6F75 6E74 4F43"            /* e IDDWRD.CountOC */
	$"4E54 042A 2A2A 2A4C 5354 430B 7379 7320"            /* NT.****LSTC.sys  */
	$"6C61 6E67 2049 4444 5752 441E 6C6F 6361"            /* lang IDDWRD.loca */
	$"6C20 7265 7320 4944 2028 6F66 6673 6574"            /* l res ID (offset */
	$"2066 726F 6D20 3530 3030 4457 5244 1032"            /*  from 5000DWRD.2 */
	$"2D62 7974 6520 6C61 6E67 7561 6765 3F44"            /* -byte language?D */
	$"5752 4404 2A2A 2A2A 4C53 5445"                      /* WRD.****LSTE */
};

data 'LPic' (5000) {
	$"0000 0002 0000 0000 0000 0000 0004 0000"            /* ................ */
};

data 'STR#' (5000, "English buttons") {
	$"0006 0D45 6E67 6C69 7368 2074 6573 7431"            /* ...English test1 */
	$"0541 6772 6565 0844 6973 6167 7265 6505"            /* .Agree.Disagree. */
	$"5072 696E 7407 5361 7665 2E2E 2E7A 4966"            /* Print.Save...zIf */
	$"2079 6F75 2061 6772 6565 2077 6974 6820"            /*  you agree with  */
	$"7468 6520 7465 726D 7320 6F66 2074 6869"            /* the terms of thi */
	$"7320 6C69 6365 6E73 652C 2063 6C69 636B"            /* s license, click */
	$"2022 4167 7265 6522 2074 6F20 6163 6365"            /*  "Agree" to acce */
	$"7373 2074 6865 2073 6F66 7477 6172 652E"            /* ss the software. */
	$"2020 4966 2079 6F75 2064 6F20 6E6F 7420"            /*   If you do not  */
	$"6167 7265 652C 2070 7265 7373 2022 4469"            /* agree, press "Di */
	$"7361 6772 6565 2E22"                                /* sagree." */
};

data 'STR#' (5002, "English") {
	$"0006 0745 6E67 6C69 7368 0541 6772 6565"            /* ...English.Agree */
	$"0844 6973 6167 7265 6505 5072 696E 7407"            /* .Disagree.Print. */
	$"5361 7665 2E2E 2E7B 4966 2079 6F75 2061"            /* Save...{If you a */
	$"6772 6565 2077 6974 6820 7468 6520 7465"            /* gree with the te */
	$"726D 7320 6F66 2074 6869 7320 6C69 6365"            /* rms of this lice */
	$"6E73 652C 2070 7265 7373 2022 4167 7265"            /* nse, press "Agre */
	$"6522 2074 6F20 696E 7374 616C 6C20 7468"            /* e" to install th */
	$"6520 736F 6674 7761 7265 2E20 2049 6620"            /* e software.  If  */
	$"796F 7520 646F 206E 6F74 2061 6772 6565"            /* you do not agree */
	$"2C20 7072 6573 7320 2244 6973 6167 7265"            /* , press "Disagre */
	$"6522 2E"                                            /* e". */
};

data 'TEXT' (5000, "English SLA") {
	$"454E 474C 4953 480A 0A41 4E53 4341 2049"            /* ENGLISHÂÂANSCA I */
	$"4E43 2E0A 4245 5441 2053 4F46 5457 4152"            /* NC.ÂBETA SOFTWAR */
	$"4520 4C49 4345 4E53 4520 4147 5245 454D"            /* E LICENSE AGREEM */
	$"454E 540A 0A50 4C45 4153 4520 5245 4144"            /* ENTÂÂPLEASE READ */
	$"2054 4849 5320 534F 4654 5741 5245 204C"            /*  THIS SOFTWARE L */
	$"4943 454E 5345 2041 4752 4545 4D45 4E54"            /* ICENSE AGREEMENT */
	$"2028 224C 4943 454E 5345 2220 4153 2044"            /*  ("LICENSE" AS D */
	$"4546 494E 4544 2042 454C 4F57 2920 4341"            /* EFINED BELOW) CA */
	$"5245 4655 4C4C 5920 4245 464F 5245 2055"            /* REFULLY BEFORE U */
	$"5349 4E47 2054 4845 2044 4556 454C 4F50"            /* SING THE DEVELOP */
	$"4552 2053 4F46 5457 4152 4520 2844 4546"            /* ER SOFTWARE (DEF */
	$"494E 4544 2042 454C 4F57 292E 2042 5920"            /* INED BELOW). BY  */
	$"5553 494E 4720 5448 4520 4445 5645 4C4F"            /* USING THE DEVELO */
	$"5045 5220 534F 4654 5741 5245 2C20 594F"            /* PER SOFTWARE, YO */
	$"5520 4152 4520 4147 5245 4549 4E47 2054"            /* U ARE AGREEING T */
	$"4F20 4245 2042 4F55 4E44 2042 5920 5448"            /* O BE BOUND BY TH */
	$"4520 5445 524D 5320 4F46 2054 4849 5320"            /* E TERMS OF THIS  */
	$"4C49 4345 4E53 452E 2049 4620 594F 5520"            /* LICENSE. IF YOU  */
	$"444F 204E 4F54 2041 4752 4545 2054 4F20"            /* DO NOT AGREE TO  */
	$"5448 4520 5445 524D 5320 4F46 2054 4849"            /* THE TERMS OF THI */
	$"5320 4C49 4345 4E53 452C 2044 4F20 4E4F"            /* S LICENSE, DO NO */
	$"5420 5553 4520 5448 4520 534F 4654 5741"            /* T USE THE SOFTWA */
	$"5245 2E20 0A0A 494D 504F 5254 414E 5420"            /* RE. ÂÂIMPORTANT  */
	$"4E4F 5445 3A20 5468 6973 2073 6F66 7477"            /* NOTE: This softw */
	$"6172 6520 6D61 7920 6265 2075 7365 6420"            /* are may be used  */
	$"746F 2072 6570 726F 6475 6365 2C20 6D6F"            /* to reproduce, mo */
	$"6469 6679 2C20 7075 626C 6973 6820 616E"            /* dify, publish an */
	$"6420 6469 7374 7269 6275 7465 206D 6174"            /* d distribute mat */
	$"6572 6961 6C73 2E20 4974 2069 7320 6C69"            /* erials. It is li */
	$"6365 6E73 6564 2074 6F20 596F 7520 6F6E"            /* censed to You on */
	$"6C79 2066 6F72 2072 6570 726F 6475 6374"            /* ly for reproduct */
	$"696F 6E2C 206D 6F64 6966 6963 6174 696F"            /* ion, modificatio */
	$"6E2C 2070 7562 6C69 6361 7469 6F6E 2061"            /* n, publication a */
	$"6E64 2064 6973 7472 6962 7574 696F 6E20"            /* nd distribution  */
	$"6F66 206E 6F6E 2D63 6F70 7972 6967 6874"            /* of non-copyright */
	$"6564 206D 6174 6572 6961 6C73 2C20 6D61"            /* ed materials, ma */
	$"7465 7269 616C 7320 696E 2077 6869 6368"            /* terials in which */
	$"2059 6F75 206F 776E 2074 6865 2063 6F70"            /*  You own the cop */
	$"7972 6967 6874 2C20 6F72 206D 6174 6572"            /* yright, or mater */
	$"6961 6C73 2059 6F75 2061 7265 2061 7574"            /* ials You are aut */
	$"686F 7269 7A65 6420 6F72 206C 6567 616C"            /* horized or legal */
	$"6C79 2070 6572 6D69 7474 6564 2074 6F20"            /* ly permitted to  */
	$"7265 7072 6F64 7563 652C 206D 6F64 6966"            /* reproduce, modif */
	$"792C 2070 7562 6C69 7368 2061 6E64 2064"            /* y, publish and d */
	$"6973 7472 6962 7574 652E 2049 6620 596F"            /* istribute. If Yo */
	$"7520 6172 6520 756E 6365 7274 6169 6E20"            /* u are uncertain  */
	$"6162 6F75 7420 596F 7572 2072 6967 6874"            /* about Your right */
	$"2074 6F20 636F 7079 2C20 6D6F 6469 6679"            /*  to copy, modify */
	$"2C20 7075 626C 6973 6820 616E 6420 6469"            /* , publish and di */
	$"7374 7269 6275 7465 2061 6E79 206D 6174"            /* stribute any mat */
	$"6572 6961 6C20 596F 7520 7368 6F75 6C64"            /* erial You should */
	$"2063 6F6E 7461 6374 2059 6F75 7220 6C65"            /*  contact Your le */
	$"6761 6C20 6164 7669 736F 722E 0A0A 312E"            /* gal advisor.ÂÂ1. */
	$"2047 656E 6572 616C 2E20 5468 6520 4465"            /*  General. The De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2028 6465 6669 6E65 6420 6265 6C6F 7729"            /*  (defined below) */
	$"2069 7320 6C69 6365 6E73 6564 2C20 6E6F"            /*  is licensed, no */
	$"7420 736F 6C64 2C20 746F 2059 6F75 2062"            /* t sold, to You b */
	$"7920 414E 5343 4120 496E 632E 2028 2241"            /* y ANSCA Inc. ("A */
	$"4E53 4341 2229 2061 6E64 2F6F 7220 414E"            /* NSCA") and/or AN */
	$"5343 4127 7320 6C69 6365 6E73 6F72 732E"            /* SCA's licensors. */
	$"2054 6865 2072 6967 6874 7320 6772 616E"            /*  The rights gran */
	$"7465 6420 6865 7265 696E 2061 7265 206C"            /* ted herein are l */
	$"696D 6974 6564 2074 6F20 414E 5343 4127"            /* imited to ANSCA' */
	$"7320 616E 642F 6F72 2041 4E53 4341 2773"            /* s and/or ANSCA's */
	$"206C 6963 656E 736F 7273 2720 7265 7370"            /*  licensors' resp */
	$"6563 7469 7665 2069 6E74 656C 6C65 6374"            /* ective intellect */
	$"7561 6C20 7072 6F70 6572 7479 2072 6967"            /* ual property rig */
	$"6874 7320 696E 2074 6865 2044 6576 656C"            /* hts in the Devel */
	$"6F70 6572 2053 6F66 7477 6172 6520 616E"            /* oper Software an */
	$"6420 646F 206E 6F74 2069 6E63 6C75 6465"            /* d do not include */
	$"2061 6E79 206F 7468 6572 2070 6174 656E"            /*  any other paten */
	$"7473 206F 7220 696E 7465 6C6C 6563 7475"            /* ts or intellectu */
	$"616C 2070 726F 7065 7274 7920 7269 6768"            /* al property righ */
	$"7473 2E20 596F 7520 6F77 6E20 7468 6520"            /* ts. You own the  */
	$"6D65 6469 6120 6F6E 2077 6869 6368 2074"            /* media on which t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 6520 6973 2072 6563 6F72 6465"            /* tware is recorde */
	$"6420 6275 7420 414E 5343 4120 616E 642F"            /* d but ANSCA and/ */
	$"6F72 2041 4E53 4341 2773 206C 6963 656E"            /* or ANSCA's licen */
	$"736F 7228 7329 2072 6574 6169 6E20 6F77"            /* sor(s) retain ow */
	$"6E65 7273 6869 7020 6F66 2074 6865 6972"            /* nership of their */
	$"2072 6573 7065 6374 6976 6520 706F 7274"            /*  respective port */
	$"696F 6E73 206F 6620 7468 6520 4465 7665"            /* ions of the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2069"            /* loper Software i */
	$"7473 656C 662E 2054 6865 2074 6572 6D73"            /* tself. The terms */
	$"206F 6620 7468 6973 204C 6963 656E 7365"            /*  of this License */
	$"2077 696C 6C20 676F 7665 726E 2061 6E79"            /*  will govern any */
	$"2073 6F66 7477 6172 6520 7570 6772 6164"            /*  software upgrad */
	$"6573 2070 726F 7669 6465 6420 6279 2041"            /* es provided by A */
	$"4E53 4341 2074 6861 7420 7265 706C 6163"            /* NSCA that replac */
	$"6520 616E 642F 6F72 2073 7570 706C 656D"            /* e and/or supplem */
	$"656E 7420 7468 6520 6F72 6967 696E 616C"            /* ent the original */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652C 2075 6E6C 6573 7320 7375 6368"            /* are, unless such */
	$"2075 7067 7261 6465 2069 7320 6163 636F"            /*  upgrade is acco */
	$"6D70 616E 6965 6420 6279 2061 2073 6570"            /* mpanied by a sep */
	$"6172 6174 6520 6C69 6365 6E73 6520 696E"            /* arate license in */
	$"2077 6869 6368 2063 6173 6520 7468 6520"            /*  which case the  */
	$"7465 726D 7320 6F66 2074 6861 7420 6C69"            /* terms of that li */
	$"6365 6E73 6520 7769 6C6C 2067 6F76 6572"            /* cense will gover */
	$"6E2E 2041 4C4C 2052 4947 4854 5320 4E4F"            /* n. ALL RIGHTS NO */
	$"5420 4558 5052 4553 534C 5920 4752 414E"            /* T EXPRESSLY GRAN */
	$"5445 4420 4845 5245 554E 4445 5220 4152"            /* TED HEREUNDER AR */
	$"4520 5245 5345 5256 4544 2054 4F20 414E"            /* E RESERVED TO AN */
	$"5343 412E 0A0A 412E 2041 6363 6570 7461"            /* SCA.ÂÂA. Accepta */
	$"6E63 652E 2049 6E20 6F72 6465 7220 746F"            /* nce. In order to */
	$"2075 7365 2074 6865 2044 6576 656C 6F70"            /*  use the Develop */
	$"6572 2053 6F66 7477 6172 652C 2059 6F75"            /* er Software, You */
	$"206D 7573 7420 6669 7273 7420 6167 7265"            /*  must first agre */
	$"6520 746F 2074 6869 7320 4C69 6365 6E73"            /* e to this Licens */
	$"652E 2020 4966 2059 6F75 2064 6F20 6E6F"            /* e.  If You do no */
	$"7420 6F72 2063 616E 6E6F 7420 6167 7265"            /* t or cannot agre */
	$"6520 746F 2074 6869 7320 4C69 6365 6E73"            /* e to this Licens */
	$"652C 2059 6F75 2061 7265 206E 6F74 2070"            /* e, You are not p */
	$"6572 6D69 7474 6564 2074 6F20 7573 6520"            /* ermitted to use  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2E20 446F 206E 6F74 2064"            /* ftware. Do not d */
	$"6F77 6E6C 6F61 6420 6F72 2075 7365 2074"            /* ownload or use t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 6520 696E 2074 6861 7420 6361"            /* tware in that ca */
	$"7365 2E20 596F 7520 6163 6365 7074 2061"            /* se. You accept a */
	$"6E64 2061 6772 6565 2074 6F20 7468 6520"            /* nd agree to the  */
	$"7465 726D 7320 6F66 2074 6869 7320 4C69"            /* terms of this Li */
	$"6365 6E73 6520 6F6E 2059 6F75 7220 6F77"            /* cense on Your ow */
	$"6E20 6265 6861 6C66 2061 6E64 2F6F 7220"            /* n behalf and/or  */
	$"6F6E 2062 6568 616C 6620 6F66 2059 6F75"            /* on behalf of You */
	$"7220 636F 6D70 616E 792C 206F 7267 616E"            /* r company, organ */
	$"697A 6174 696F 6E20 6F72 2065 6475 6361"            /* ization or educa */
	$"7469 6F6E 616C 2069 6E73 7469 7475 7469"            /* tional instituti */
	$"6F6E 2061 7320 6974 7320 6175 7468 6F72"            /* on as its author */
	$"697A 6564 206C 6567 616C 2072 6570 7265"            /* ized legal repre */
	$"7365 6E74 6174 6976 652C 2062 7920 646F"            /* sentative, by do */
	$"696E 6720 6569 7468 6572 206F 6620 7468"            /* ing either of th */
	$"6520 666F 6C6C 6F77 696E 673A 200A 2861"            /* e following: Â(a */
	$"2920 6368 6563 6B69 6E67 2074 6865 2062"            /* ) checking the b */
	$"6F78 2064 6973 706C 6179 6564 2061 7420"            /* ox displayed at  */
	$"7468 6520 656E 6420 6F66 2074 6869 7320"            /* the end of this  */
	$"4167 7265 656D 656E 7420 6966 2059 6F75"            /* Agreement if You */
	$"2061 7265 2072 6561 6469 6E67 2074 6869"            /*  are reading thi */
	$"7320 6F6E 2061 6E20 414E 5343 4120 7765"            /* s on an ANSCA we */
	$"6273 6974 653B 206F 7220 0A28 6229 2063"            /* bsite; or Â(b) c */
	$"6C69 636B 696E 6720 616E 2022 4167 7265"            /* licking an "Agre */
	$"6522 206F 7220 7369 6D69 6C61 7220 6275"            /* e" or similar bu */
	$"7474 6F6E 2C20 7768 6572 6520 7468 6973"            /* tton, where this */
	$"206F 7074 696F 6E20 6973 2070 726F 7669"            /*  option is provi */
	$"6465 6420 6279 2041 4E53 4341 2E20 0A0A"            /* ded by ANSCA. ÂÂ */
	$"422E 2044 6566 696E 6974 696F 6E73 2E20"            /* B. Definitions.  */
	$"0A0A 2244 6576 656C 6F70 6572 2053 6F66"            /* ÂÂ"Developer Sof */
	$"7477 6172 6522 2063 6F6C 6C65 6374 6976"            /* tware" collectiv */
	$"656C 7920 6D65 616E 733A 2028 6129 2074"            /* ely means: (a) t */
	$"6865 2053 444B 2028 6465 6669 6E65 6420"            /* he SDK (defined  */
	$"6265 6C6F 7729 2061 6E64 2028 6229 2074"            /* below) and (b) t */
	$"6865 2041 7070 6C69 6361 7469 6F6E 7320"            /* he Applications  */
	$"2864 6566 696E 6564 2062 656C 6F77 2920"            /* (defined below)  */
	$"6372 6561 7465 6420 616E 6420 7072 6F64"            /* created and prod */
	$"7563 6564 2062 7920 7468 6520 5344 4B2C"            /* uced by the SDK, */
	$"2061 6E64 2069 6E63 6C75 6465 7320 616E"            /*  and includes an */
	$"7920 5570 6461 7465 7320 746F 2061 6E79"            /* y Updates to any */
	$"206F 6620 7468 6520 666F 7265 676F 696E"            /*  of the foregoin */
	$"6720 7468 6174 206D 6179 2062 6520 7072"            /* g that may be pr */
	$"6F76 6964 6564 2062 7920 414E 5343 412E"            /* ovided by ANSCA. */
	$"200A 0A22 4170 706C 6963 6174 696F 6E22"            /*  ÂÂ"Application" */
	$"206D 6561 6E73 2074 6865 2073 6F66 7477"            /*  means the softw */
	$"6172 6520 7072 6F67 7261 6D20 7468 6174"            /* are program that */
	$"2072 756E 7320 6F6E 2061 6E20 414E 5343"            /*  runs on an ANSC */
	$"412D 7375 7070 6F72 7465 6420 6465 7669"            /* A-supported devi */
	$"6365 2061 6E64 2074 6861 7420 6973 2070"            /* ce and that is p */
	$"726F 6475 6365 6420 6279 2074 6865 2053"            /* roduced by the S */
	$"444B 2075 7369 6E67 2073 6F75 7263 6520"            /* DK using source  */
	$"636F 6465 2C20 696D 6167 6573 2C20 736F"            /* code, images, so */
	$"756E 6473 2C20 616E 6420 6F74 6865 7220"            /* unds, and other  */
	$"6D65 6469 6120 7072 6F64 7563 6564 2062"            /* media produced b */
	$"7920 596F 7520 696E 2063 6F6D 706C 6961"            /* y You in complia */
	$"6E63 6520 7769 7468 2074 6865 2044 6F63"            /* nce with the Doc */
	$"756D 656E 7461 7469 6F6E 2028 6465 6669"            /* umentation (defi */
	$"6E65 6420 6265 6C6F 7729 2061 6E64 2074"            /* ned below) and t */
	$"6865 2074 6572 6D73 206F 6620 7468 6973"            /* he terms of this */
	$"204C 6963 656E 7365 2C20 756E 6465 7220"            /*  License, under  */
	$"596F 7572 206F 776E 2074 7261 6465 6D61"            /* Your own tradema */
	$"726B 206F 7220 6272 616E 642C 2061 6E64"            /* rk or brand, and */
	$"2066 6F72 2073 7065 6369 6669 6320 7573"            /*  for specific us */
	$"6520 7769 7468 2041 4E53 4341 2D73 7570"            /* e with ANSCA-sup */
	$"706F 7274 6564 2064 6576 6963 6573 2C20"            /* ported devices,  */
	$"696E 636C 7564 696E 6720 6275 6720 6669"            /* including bug fi */
	$"7865 732C 2075 7064 6174 6573 2C20 7570"            /* xes, updates, up */
	$"6772 6164 6573 2C20 6D6F 6469 6669 6361"            /* grades, modifica */
	$"7469 6F6E 732C 2065 6E68 616E 6365 6D65"            /* tions, enhanceme */
	$"6E74 732C 2073 7570 706C 656D 656E 7473"            /* nts, supplements */
	$"2074 6F2C 2072 6576 6973 696F 6E73 2C20"            /*  to, revisions,  */
	$"6E65 7720 7265 6C65 6173 6573 2061 6E64"            /* new releases and */
	$"206E 6577 2076 6572 7369 6F6E 7320 6F66"            /*  new versions of */
	$"2073 7563 6820 736F 6674 7761 7265 2070"            /*  such software p */
	$"726F 6772 616D 732E 0A0A 2253 444B 2220"            /* rograms.ÂÂ"SDK"  */
	$"2853 6F66 7477 6172 6520 4465 7665 6C6F"            /* (Software Develo */
	$"706D 656E 7420 4B69 7429 206D 6561 6E73"            /* pment Kit) means */
	$"2074 6865 2044 6F63 756D 656E 7461 7469"            /*  the Documentati */
	$"6F6E 2C20 736F 6674 7761 7265 2028 736F"            /* on, software (so */
	$"7572 6365 2063 6F64 6520 616E 6420 6F62"            /* urce code and ob */
	$"6A65 6374 2063 6F64 6529 2C20 4170 706C"            /* ject code), Appl */
	$"6963 6174 696F 6E73 2C20 7361 6D70 6C65"            /* ications, sample */
	$"206F 7220 6578 616D 706C 6520 636F 6465"            /*  or example code */
	$"2C20 7369 6D75 6C61 746F 722C 2074 6F6F"            /* , simulator, too */
	$"6C73 2C20 7574 696C 6974 6965 732C 206C"            /* ls, utilities, l */
	$"6962 7261 7269 6573 2C20 4150 4973 2C20"            /* ibraries, APIs,  */
	$"6461 7461 2C20 6669 6C65 732C 2061 6E64"            /* data, files, and */
	$"206F 7468 6572 206D 6174 6572 6961 6C73"            /*  other materials */
	$"2070 726F 7669 6465 6420 6F72 206D 6164"            /*  provided or mad */
	$"6520 6176 6169 6C61 626C 6520 6279 2041"            /* e available by A */
	$"4E53 4341 2066 6F72 2075 7365 2062 7920"            /* NSCA for use by  */
	$"596F 7520 696E 2063 6F6E 6E65 6374 696F"            /* You in connectio */
	$"6E20 7769 7468 2059 6F75 7220 4170 706C"            /* n with Your Appl */
	$"6963 6174 696F 6E20 6465 7665 6C6F 706D"            /* ication developm */
	$"656E 7420 7768 6574 6865 7220 6F6E 2064"            /* ent whether on d */
	$"6973 6B2C 206F 6E20 7072 696E 7420 6F72"            /* isk, on print or */
	$"2065 6C65 6374 726F 6E69 6320 646F 6375"            /*  electronic docu */
	$"6D65 6E74 6174 696F 6E2C 2069 6E20 7265"            /* mentation, in re */
	$"6164 206F 6E6C 7920 6D65 6D6F 7279 2C20"            /* ad only memory,  */
	$"6F6E 2061 6E79 206F 7468 6572 206D 6564"            /* on any other med */
	$"6961 2C20 6F72 206D 6164 6520 6176 6169"            /* ia, or made avai */
	$"6C61 626C 6520 6F6E 2061 2073 6572 7665"            /* lable on a serve */
	$"7220 6F72 206F 7468 6572 7769 7365 2070"            /* r or otherwise p */
	$"726F 7669 6465 6420 6173 2061 6E20 6F6E"            /* rovided as an on */
	$"6C69 6E65 2073 6572 7669 6365 2C20 616E"            /* line service, an */
	$"6420 696E 636C 7564 6573 2061 6E79 2055"            /* d includes any U */
	$"7064 6174 6573 2074 6861 7420 6D61 7920"            /* pdates that may  */
	$"6265 2070 726F 7669 6465 6420 6F72 206D"            /* be provided or m */
	$"6164 6520 6176 6169 6C61 626C 6520 6279"            /* ade available by */
	$"2041 4E53 4341 2E0A 0A22 446F 6375 6D65"            /*  ANSCA.ÂÂ"Docume */
	$"6E74 6174 696F 6E22 206D 6561 6E73 2061"            /* ntation" means a */
	$"6E79 2074 6563 686E 6963 616C 206F 7220"            /* ny technical or  */
	$"6F74 6865 7220 7370 6563 6966 6963 6174"            /* other specificat */
	$"696F 6E73 206F 7220 646F 6375 6D65 6E74"            /* ions or document */
	$"6174 696F 6E20 7468 6174 2041 4E53 4341"            /* ation that ANSCA */
	$"206D 6179 206D 616B 6520 6176 6169 6C61"            /*  may make availa */
	$"626C 6520 6F72 2070 726F 7669 6465 2074"            /* ble or provide t */
	$"6F20 596F 7520 7265 6C61 7469 6E67 2074"            /* o You relating t */
	$"6F20 6F72 2066 6F72 2075 7365 2069 6E20"            /* o or for use in  */
	$"0A63 6F6E 6E65 6374 696F 6E20 7769 7468"            /* Âconnection with */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 652E 0A0A 224C 6963 656E"            /* oftware.ÂÂ"Licen */
	$"7365 222C 2022 4C69 6365 6E73 6520 4167"            /* se", "License Ag */
	$"7265 656D 656E 7422 2C20 616E 6420 2241"            /* reement", and "A */
	$"6772 6565 6D65 6E74 2220 6D65 616E 7320"            /* greement" means  */
	$"616E 6420 7265 6665 7273 2074 6F20 7468"            /* and refers to th */
	$"6973 2053 6F66 7477 6172 6520 4C69 6365"            /* is Software Lice */
	$"6E73 6520 4167 7265 656D 656E 742E 0A0A"            /* nse Agreement.ÂÂ */
	$"2255 7064 6174 6573 2220 6D65 616E 7320"            /* "Updates" means  */
	$"6275 6720 6669 7865 732C 2075 7064 6174"            /* bug fixes, updat */
	$"6573 2C20 7570 6772 6164 6573 2C20 6D6F"            /* es, upgrades, mo */
	$"6469 6669 6361 7469 6F6E 732C 2065 6E68"            /* difications, enh */
	$"616E 6365 6D65 6E74 732C 2073 7570 706C"            /* ancements, suppl */
	$"656D 656E 7473 2C20 616E 6420 6E65 7720"            /* ements, and new  */
	$"7265 6C65 6173 6573 206F 7220 7665 7273"            /* releases or vers */
	$"696F 6E73 206F 6620 7468 6520 4465 7665"            /* ions of the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2C20"            /* loper Software,  */
	$"6F72 2074 6F20 616E 7920 7061 7274 206F"            /* or to any part o */
	$"6620 7468 6520 4465 7665 6C6F 7065 7220"            /* f the Developer  */
	$"536F 6674 7761 7265 2E0A 0A22 596F 7522"            /* Software.ÂÂ"You" */
	$"2C20 2259 6F75 7222 2061 6E64 2022 4C69"            /* , "Your" and "Li */
	$"6365 6E73 6565 2220 6D65 616E 7320 616E"            /* censee" means an */
	$"6420 7265 6665 7273 2074 6F20 7468 6520"            /* d refers to the  */
	$"7065 7273 6F6E 2873 2920 6F72 206C 6567"            /* person(s) or leg */
	$"616C 2065 6E74 6974 7920 7573 696E 6720"            /* al entity using  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 206F 7220 6F74 6865 7277"            /* ftware or otherw */
	$"6973 6520 6578 6572 6369 7369 6E67 2072"            /* ise exercising r */
	$"6967 6874 7320 756E 6465 7220 7468 6973"            /* ights under this */
	$"2041 6772 6565 6D65 6E74 2E20 2049 6620"            /*  Agreement.  If  */
	$"596F 7520 6172 6520 656E 7465 7269 6E67"            /* You are entering */
	$"2069 6E74 6F20 7468 6973 2041 6772 6565"            /*  into this Agree */
	$"6D65 6E74 206F 6E20 6265 6861 6C66 206F"            /* ment on behalf o */
	$"6620 596F 7572 2063 6F6D 7061 6E79 2C20"            /* f Your company,  */
	$"6F72 6761 6E69 7A61 7469 6F6E 206F 7220"            /* organization or  */
	$"6564 7563 6174 696F 6E61 6C20 696E 7374"            /* educational inst */
	$"6974 7574 696F 6E2C 2022 596F 7522 206F"            /* itution, "You" o */
	$"7220 2259 6F75 7222 2072 6566 6572 7320"            /* r "Your" refers  */
	$"746F 2059 6F75 7220 636F 6D70 616E 792C"            /* to Your company, */
	$"206F 7267 616E 697A 6174 696F 6E20 6F72"            /*  organization or */
	$"2065 6475 6361 7469 6F6E 616C 2069 6E73"            /*  educational ins */
	$"7469 7475 7469 6F6E 2061 7320 7765 6C6C"            /* titution as well */
	$"2E0A 0A32 2E20 5065 726D 6974 7465 6420"            /* .ÂÂ2. Permitted  */
	$"4C69 6365 6E73 6520 5573 6573 2061 6E64"            /* License Uses and */
	$"2052 6573 7472 6963 7469 6F6E 732E 2020"            /*  Restrictions.   */
	$"0A0A 412E 2044 6576 656C 6F70 6572 2053"            /* ÂÂA. Developer S */
	$"6F66 7477 6172 652E 2053 7562 6A65 6374"            /* oftware. Subject */
	$"2074 6F20 7468 6520 7465 726D 7320 6265"            /*  to the terms be */
	$"6C6F 772C 2041 4E53 4341 2067 7261 6E74"            /* low, ANSCA grant */
	$"7320 596F 7520 6120 6E6F 6E2D 6578 636C"            /* s You a non-excl */
	$"7573 6976 652C 206E 6F6E 2D74 7261 6E73"            /* usive, non-trans */
	$"6665 7261 626C 6520 6C69 6365 6E73 6520"            /* ferable license  */
	$"2877 6974 686F 7574 2074 6865 2072 6967"            /* (without the rig */
	$"6874 2074 6F20 7375 626C 6963 656E 7365"            /* ht to sublicense */
	$"2920 2869 2920 746F 2075 7365 2074 6865"            /* ) (i) to use the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 6520 736F 6C65 6C79 2066 6F72 2074"            /* are solely for t */
	$"6865 2070 7572 706F 7365 206F 6620 2861"            /* he purpose of (a */
	$"2920 7465 7374 696E 6720 7468 6520 4465"            /* ) testing the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2061 6E64 2028 6229 2064 6576 656C 6F70"            /*  and (b) develop */
	$"696E 6720 4170 706C 6963 6174 696F 6E20"            /* ing Application  */
	$"666F 7220 6D6F 6269 6C65 2070 686F 6E65"            /* for mobile phone */
	$"7320 616E 6420 6F74 6865 7220 736F 6674"            /* s and other soft */
	$"7761 7265 2066 6F72 206D 6F62 696C 6520"            /* ware for mobile  */
	$"7068 6F6E 6573 3B20 2869 6929 2074 6F20"            /* phones; (ii) to  */
	$"636F 7079 2044 6576 656C 6F70 6572 2053"            /* copy Developer S */
	$"6F66 7477 6172 6520 666F 7220 6172 6368"            /* oftware for arch */
	$"6976 616C 206F 7220 6261 636B 7570 2070"            /* ival or backup p */
	$"7572 706F 7365 732C 2070 726F 7669 6465"            /* urposes, provide */
	$"6420 7468 6174 2061 6C6C 2074 6974 6C65"            /* d that all title */
	$"7320 616E 6420 7472 6164 656D 6172 6B73"            /* s and trademarks */
	$"2C20 636F 7079 7269 6768 742C 2061 6E64"            /* , copyright, and */
	$"2072 6573 7472 6963 7465 6420 7269 6768"            /*  restricted righ */
	$"7473 206E 6F74 6963 6573 2061 7265 2072"            /* ts notices are r */
	$"6570 726F 6475 6365 6420 6F6E 2073 7563"            /* eproduced on suc */
	$"6820 636F 7069 6573 3B20 616E 6420 2869"            /* h copies; and (i */
	$"6969 2920 746F 206D 6172 6B65 7420 616E"            /* ii) to market an */
	$"6420 6469 7374 7269 6275 7465 2C20 6469"            /* d distribute, di */
	$"7265 6374 6C79 206F 7220 696E 6469 7265"            /* rectly or indire */
	$"6374 6C79 2074 6872 6F75 6768 2064 6973"            /* ctly through dis */
	$"7472 6962 7574 6F72 732C 2041 7070 6C69"            /* tributors, Appli */
	$"6361 7469 6F6E 7320 6372 6561 7465 6420"            /* cations created  */
	$"7769 7468 2074 6865 2044 6576 656C 6F70"            /* with the Develop */
	$"6572 2053 6F66 7477 6172 652C 2070 726F"            /* er Software, pro */
	$"7669 6465 6420 7468 6174 2028 6129 2059"            /* vided that (a) Y */
	$"6F75 2077 696C 6C20 6E6F 742C 2061 6E64"            /* ou will not, and */
	$"2077 696C 6C20 6E6F 7420 7065 726D 6974"            /*  will not permit */
	$"2061 6E79 2074 6869 7264 2070 6172 7479"            /*  any third party */
	$"2074 6F2C 206D 6F64 6966 792C 2063 7265"            /*  to, modify, cre */
	$"6174 6520 6465 7269 7661 7469 7665 2077"            /* ate derivative w */
	$"6F72 6B73 206F 662C 2074 7261 6E73 6C61"            /* orks of, transla */
	$"7465 2C20 7265 7665 7273 6520 656E 6769"            /* te, reverse engi */
	$"6E65 6572 2C20 6465 636F 6D70 696C 652C"            /* neer, decompile, */
	$"2064 6973 6173 7365 6D62 6C65 2C20 6F72"            /*  disassemble, or */
	$"206F 7468 6572 7769 7365 2072 6564 7563"            /*  otherwise reduc */
	$"6520 746F 2068 756D 616E 2070 6572 6365"            /* e to human perce */
	$"6976 6162 6C65 2066 6F72 6D20 616E 7920"            /* ivable form any  */
	$"706F 7274 696F 6E20 6F66 2074 6865 2041"            /* portion of the A */
	$"7070 6C69 6361 7469 6F6E 2063 7265 6174"            /* pplication creat */
	$"6564 2062 7920 7468 6520 5344 4B20 616E"            /* ed by the SDK an */
	$"6420 2862 2920 616E 7920 4170 706C 6963"            /* d (b) any Applic */
	$"6174 696F 6E20 6372 6561 7465 6420 7769"            /* ation created wi */
	$"7468 2074 6865 2053 444B 2069 7320 6469"            /* th the SDK is di */
	$"7374 7269 6275 7465 6420 746F 2065 6E64"            /* stributed to end */
	$"2075 7365 7273 2075 6E64 6572 2061 6E20"            /*  users under an  */
	$"656E 666F 7263 6561 626C 6520 656E 642D"            /* enforceable end- */
	$"7573 6572 206C 6963 656E 7365 2061 6772"            /* user license agr */
	$"6565 6D65 6E74 2063 6F6E 7461 696E 696E"            /* eement containin */
	$"6720 6174 206C 6561 7374 2074 6865 2066"            /* g at least the f */
	$"6F6C 6C6F 7769 6E67 206D 696E 696D 756D"            /* ollowing minimum */
	$"2074 6572 6D73 3A0A 2A20 5072 6F68 6962"            /*  terms:Â* Prohib */
	$"6974 696F 6E20 6167 6169 6E73 7420 6469"            /* ition against di */
	$"7374 7269 6275 7469 6F6E 2061 6E64 2063"            /* stribution and c */
	$"6F70 7969 6E67 2E0A 2A20 5072 6F68 6962"            /* opying.Â* Prohib */
	$"6974 696F 6E20 6167 6169 6E73 7420 6D6F"            /* ition against mo */
	$"6469 6669 6361 7469 6F6E 7320 616E 6420"            /* difications and  */
	$"6465 7269 7661 7469 7665 2077 6F72 6B73"            /* derivative works */
	$"2E0A 2A20 5072 6F68 6962 6974 696F 6E20"            /* .Â* Prohibition  */
	$"6167 6169 6E73 7420 6465 636F 6D70 696C"            /* against decompil */
	$"696E 672C 2072 6576 6572 7365 2065 6E67"            /* ing, reverse eng */
	$"696E 6565 7269 6E67 2C20 6469 7361 7373"            /* ineering, disass */
	$"656D 626C 696E 672C 2061 6E64 206F 7468"            /* embling, and oth */
	$"6572 7769 7365 2072 6564 7563 696E 6720"            /* erwise reducing  */
	$"7468 6520 736F 6674 7761 7265 2074 6F20"            /* the software to  */
	$"6120 6875 6D61 6E2D 7065 7263 6569 7661"            /* a human-perceiva */
	$"626C 6520 666F 726D 2E0A 2A20 5072 6F76"            /* ble form.Â* Prov */
	$"6973 696F 6E20 696E 6469 6361 7469 6E67"            /* ision indicating */
	$"206F 776E 6572 7368 6970 206F 6620 736F"            /*  ownership of so */
	$"6674 7761 7265 2062 7920 596F 7520 616E"            /* ftware by You an */
	$"6420 596F 7572 2073 7570 706C 6965 7273"            /* d Your suppliers */
	$"2E0A 2A20 4469 7363 6C61 696D 6572 206F"            /* .Â* Disclaimer o */
	$"6620 616C 6C20 6170 706C 6963 6162 6C65"            /* f all applicable */
	$"2073 7461 7475 746F 7279 2077 6172 7261"            /*  statutory warra */
	$"6E74 6965 732C 2074 6F20 7468 6520 6675"            /* nties, to the fu */
	$"6C6C 2065 7874 656E 7420 616C 6C6F 7765"            /* ll extent allowe */
	$"6420 6279 206C 6177 2E0A 2A20 4C69 6D69"            /* d by law.Â* Limi */
	$"7461 7469 6F6E 206F 6620 6C69 6162 696C"            /* tation of liabil */
	$"6974 7920 6E6F 7420 746F 2065 7863 6565"            /* ity not to excee */
	$"6420 7072 6963 6520 6F66 2059 6F75 7220"            /* d price of Your  */
	$"5072 6F64 7563 742C 2061 6E64 2070 726F"            /* Product, and pro */
	$"7669 7369 6F6E 2074 6861 7420 736F 6C65"            /* vision that sole */
	$"2072 656D 6564 7920 7368 616C 6C20 6265"            /*  remedy shall be */
	$"2061 2072 6967 6874 206F 6620 7265 7475"            /*  a right of retu */
	$"726E 2061 6E64 2072 6566 756E 642C 2069"            /* rn and refund, i */
	$"6620 616E 792C 2066 726F 6D20 596F 752E"            /* f any, from You. */
	$"0A2A 2044 6973 636C 6169 6D65 7220 6F66"            /* Â* Disclaimer of */
	$"2069 6E64 6972 6563 742C 2073 7065 6369"            /*  indirect, speci */
	$"616C 2C20 696E 6369 6465 6E74 616C 2C20"            /* al, incidental,  */
	$"7075 6E69 7469 7665 2C20 616E 6420 636F"            /* punitive, and co */
	$"6E73 6571 7565 6E74 6961 6C20 6461 6D61"            /* nsequential dama */
	$"6765 732E 0A2A 204D 616B 696E 6720 414E"            /* ges.Â* Making AN */
	$"5343 4120 6120 7468 6972 642D 7061 7274"            /* SCA a third-part */
	$"7920 6265 6E65 6669 6369 6172 7920 666F"            /* y beneficiary fo */
	$"7220 7075 7270 6F73 6573 206F 6620 656E"            /* r purposes of en */
	$"666F 7263 696E 6720 6974 7320 696E 7465"            /* forcing its inte */
	$"6C6C 6563 7475 616C 2070 726F 7065 7274"            /* llectual propert */
	$"7920 7269 6768 7473 206F 7220 616E 7920"            /* y rights or any  */
	$"6469 7363 6C61 696D 6572 7320 6F72 206C"            /* disclaimers or l */
	$"696D 6974 6174 696F 6E73 2E0A 0A42 2E20"            /* imitations.ÂÂB.  */
	$"4F70 656E 2D53 6F75 7263 6564 2053 6F66"            /* Open-Sourced Sof */
	$"7477 6172 652E 2054 6865 2074 6572 6D73"            /* tware. The terms */
	$"2061 6E64 2063 6F6E 6469 7469 6F6E 7320"            /*  and conditions  */
	$"6F66 2074 6869 7320 4C69 6365 6E73 6520"            /* of this License  */
	$"7368 616C 6C20 6E6F 7420 6170 706C 7920"            /* shall not apply  */
	$"746F 2061 6E79 204F 7065 6E20 536F 7572"            /* to any Open Sour */
	$"6365 2053 6F66 7477 6172 6520 6163 636F"            /* ce Software acco */
	$"6D70 616E 7969 6E67 2074 6865 2044 6576"            /* mpanying the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 652E"            /* eloper Software. */
	$"2041 6E79 2073 7563 6820 4F70 656E 2053"            /*  Any such Open S */
	$"6F75 7263 6520 536F 6674 7761 7265 2069"            /* ource Software i */
	$"7320 7072 6F76 6964 6564 2075 6E64 6572"            /* s provided under */
	$"2074 6865 2074 6572 6D73 206F 6620 7468"            /*  the terms of th */
	$"6520 6F70 656E 2073 6F75 7263 6520 6C69"            /* e open source li */
	$"6365 6E73 6520 6167 7265 656D 656E 7420"            /* cense agreement  */
	$"6F72 2063 6F70 7972 6967 6874 206E 6F74"            /* or copyright not */
	$"6963 6520 6163 636F 6D70 616E 7969 6E67"            /* ice accompanying */
	$"2073 7563 6820 4F70 656E 2053 6F75 7263"            /*  such Open Sourc */
	$"6520 536F 6674 7761 7265 206F 7220 696E"            /* e Software or in */
	$"2074 6865 206F 7065 6E20 736F 7572 6365"            /*  the open source */
	$"206C 6963 656E 7365 7320 6669 6C65 2061"            /*  licenses file a */
	$"6363 6F6D 7061 6E79 696E 6720 7468 6520"            /* ccompanying the  */
	$"4465 7665 6C6F 7065 7220 536F 6674 7761"            /* Developer Softwa */
	$"7265 2E20 0A0A 432E 2053 616D 706C 6520"            /* re. ÂÂC. Sample  */
	$"436F 6465 2E20 4365 7274 6169 6E20 706F"            /* Code. Certain po */
	$"7274 696F 6E73 206F 6620 7468 6520 4465"            /* rtions of the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2063 6F6E 7369 7374 206F 6620 7361 6D70"            /*  consist of samp */
	$"6C65 206F 7220 6578 616D 706C 6520 636F"            /* le or example co */
	$"6465 2070 726F 7669 6465 6420 6279 2041"            /* de provided by A */
	$"4E53 4341 2028 2253 616D 706C 6520 436F"            /* NSCA ("Sample Co */
	$"6465 2229 2E20 2059 6F75 206D 6179 2075"            /* de").  You may u */
	$"7365 2C20 7265 7072 6F64 7563 652C 206D"            /* se, reproduce, m */
	$"6F64 6966 7920 616E 6420 7265 6469 7374"            /* odify and redist */
	$"7269 6275 7465 2073 7563 6820 5361 6D70"            /* ribute such Samp */
	$"6C65 2043 6F64 6520 696E 2061 6363 6F72"            /* le Code in accor */
	$"6461 6E63 6520 7769 7468 2074 6865 206C"            /* dance with the l */
	$"6963 656E 7369 6E67 2074 6572 6D73 2061"            /* icensing terms a */
	$"6363 6F6D 7061 6E79 696E 6720 7375 6368"            /* ccompanying such */
	$"2053 616D 706C 6520 436F 6465 206F 7220"            /*  Sample Code or  */
	$"7265 6C61 7465 6420 7072 6F6A 6563 7428"            /* related project( */
	$"7329 2E0A 0A44 2E20 4576 616C 7561 7469"            /* s).ÂÂD. Evaluati */
	$"6F6E 2046 6565 6462 6163 6B2E 2054 6865"            /* on Feedback. The */
	$"2070 7572 706F 7365 206F 6620 7468 6973"            /*  purpose of this */
	$"206C 696D 6974 6564 206C 6963 656E 7365"            /*  limited license */
	$"2069 7320 7468 6520 7465 7374 696E 6720"            /*  is the testing  */
	$"616E 6420 6576 616C 7561 7469 6F6E 206F"            /* and evaluation o */
	$"6620 7468 6520 4465 7665 6C6F 7065 7220"            /* f the Developer  */
	$"536F 6674 7761 7265 2E20 496E 2066 7572"            /* Software. In fur */
	$"7468 6572 616E 6365 206F 6620 7468 6973"            /* therance of this */
	$"2070 7572 706F 7365 2C20 596F 7520 7368"            /*  purpose, You sh */
	$"616C 6C20 7072 6F76 6964 6520 6665 6564"            /* all provide feed */
	$"6261 636B 2074 6F20 414E 5343 4120 636F"            /* back to ANSCA co */
	$"6E63 6572 6E69 6E67 2074 6865 2066 756E"            /* ncerning the fun */
	$"6374 696F 6E61 6C69 7479 2061 6E64 2070"            /* ctionality and p */
	$"6572 666F 726D 616E 6365 206F 6620 7468"            /* erformance of th */
	$"6520 4465 7665 6C6F 7065 7220 536F 6674"            /* e Developer Soft */
	$"7761 7265 2066 726F 6D20 7469 6D65 2074"            /* ware from time t */
	$"6F20 7469 6D65 2061 7320 7265 6173 6F6E"            /* o time as reason */
	$"6162 6C79 2072 6571 7565 7374 6564 2062"            /* ably requested b */
	$"7920 414E 5343 412C 2069 6E63 6C75 6469"            /* y ANSCA, includi */
	$"6E67 2C20 7769 7468 6F75 7420 6C69 6D69"            /* ng, without limi */
	$"7461 7469 6F6E 2C20 6964 656E 7469 6679"            /* tation, identify */
	$"696E 6720 706F 7465 6E74 6961 6C20 6572"            /* ing potential er */
	$"726F 7273 2061 6E64 2069 6D70 726F 7665"            /* rors and improve */
	$"6D65 6E74 732E 2053 7563 6820 6665 6564"            /* ments. Such feed */
	$"6261 636B 2077 696C 6C20 6265 2069 6E20"            /* back will be in  */
	$"6120 6D61 6E6E 6572 2063 6F6E 7665 6E69"            /* a manner conveni */
	$"656E 7420 746F 2059 6F75 2061 6E64 2077"            /* ent to You and w */
	$"696C 6C20 6265 2073 7562 6A65 6374 2074"            /* ill be subject t */
	$"6F20 7265 6173 6F6E 6162 6C65 2061 7661"            /* o reasonable ava */
	$"696C 6162 696C 6974 7920 6F66 2059 6F75"            /* ilability of You */
	$"7220 7065 7273 6F6E 6E65 6C2E 204E 6F74"            /* r personnel. Not */
	$"7769 7468 7374 616E 6469 6E67 2074 6865"            /* withstanding the */
	$"2066 6F72 6567 6F69 6E67 2C20 7072 696F"            /*  foregoing, prio */
	$"7220 746F 2059 6F75 7220 6469 7363 6C6F"            /* r to Your disclo */
	$"7369 6E67 2074 6F20 414E 5343 4120 616E"            /* sing to ANSCA an */
	$"7920 696E 666F 726D 6174 696F 6E20 696E"            /* y information in */
	$"2063 6F6E 6E65 6374 696F 6E20 7769 7468"            /*  connection with */
	$"2074 6869 7320 4C69 6365 6E73 6520 7768"            /*  this License wh */
	$"6963 6820 596F 7520 636F 6E73 6964 6572"            /* ich You consider */
	$"2070 726F 7072 6965 7461 7279 206F 7220"            /*  proprietary or  */
	$"636F 6E66 6964 656E 7469 616C 2C20 596F"            /* confidential, Yo */
	$"7520 7368 616C 6C20 6F62 7461 696E 2041"            /* u shall obtain A */
	$"4E53 4341 2773 2070 7269 6F72 2077 7269"            /* NSCA's prior wri */
	$"7474 656E 2061 7070 726F 7661 6C20 746F"            /* tten approval to */
	$"2064 6973 636C 6F73 6520 7375 6368 2069"            /*  disclose such i */
	$"6E66 6F72 6D61 7469 6F6E 2074 6F20 414E"            /* nformation to AN */
	$"5343 412C 2061 6E64 2077 6974 686F 7574"            /* SCA, and without */
	$"2073 7563 6820 7072 696F 7220 7772 6974"            /*  such prior writ */
	$"7465 6E20 6170 7072 6F76 616C 2066 726F"            /* ten approval fro */
	$"6D20 414E 5343 412C 2059 6F75 2073 6861"            /* m ANSCA, You sha */
	$"6C6C 206E 6F74 2064 6973 636C 6F73 6520"            /* ll not disclose  */
	$"616E 7920 7375 6368 2069 6E66 6F72 6D61"            /* any such informa */
	$"7469 6F6E 2074 6F20 414E 5343 412E 2046"            /* tion to ANSCA. F */
	$"6565 6462 6163 6B20 616E 6420 6F74 6865"            /* eedback and othe */
	$"7220 696E 666F 726D 6174 696F 6E20 7768"            /* r information wh */
	$"6963 6820 6973 2070 726F 7669 6465 6420"            /* ich is provided  */
	$"6279 2059 6F75 2074 6F20 414E 5343 4120"            /* by You to ANSCA  */
	$"696E 2063 6F6E 6E65 6374 696F 6E20 7769"            /* in connection wi */
	$"7468 2074 6865 2044 6576 656C 6F70 6572"            /* th the Developer */
	$"2053 6F66 7477 6172 6520 6F72 2074 6869"            /*  Software or thi */
	$"7320 4C69 6365 6E73 6520 6D61 7920 6265"            /* s License may be */
	$"2075 7365 6420 6279 2041 4E53 4341 2074"            /*  used by ANSCA t */
	$"6F20 696D 7072 6F76 6520 6F72 2065 6E68"            /* o improve or enh */
	$"616E 6365 2069 7473 2070 726F 6475 6374"            /* ance its product */
	$"7320 616E 642C 2061 6363 6F72 6469 6E67"            /* s and, according */
	$"6C79 2C20 414E 5343 4120 7368 616C 6C20"            /* ly, ANSCA shall  */
	$"6861 7665 2061 206E 6F6E 2D65 7863 6C75"            /* have a non-exclu */
	$"7369 7665 2C20 7065 7270 6574 7561 6C2C"            /* sive, perpetual, */
	$"2069 7272 6576 6F63 6162 6C65 2C20 726F"            /*  irrevocable, ro */
	$"7961 6C74 792D 6672 6565 2C20 776F 726C"            /* yalty-free, worl */
	$"6477 6964 6520 7269 6768 7420 616E 6420"            /* dwide right and  */
	$"6C69 6365 6E73 6520 746F 2075 7365 2C20"            /* license to use,  */
	$"7265 7072 6F64 7563 652C 2064 6973 636C"            /* reproduce, discl */
	$"6F73 652C 2073 7562 6C69 6365 6E73 652C"            /* ose, sublicense, */
	$"2064 6973 7472 6962 7574 652C 206D 6F64"            /*  distribute, mod */
	$"6966 792C 2061 6E64 206F 7468 6572 7769"            /* ify, and otherwi */
	$"7365 2065 7870 6C6F 6974 2073 7563 6820"            /* se exploit such  */
	$"6665 6564 6261 636B 2061 6E64 2069 6E66"            /* feedback and inf */
	$"6F72 6D61 7469 6F6E 2077 6974 686F 7574"            /* ormation without */
	$"2072 6573 7472 6963 7469 6F6E 2E20 E280"            /*  restriction. â€ */
	$"A80A 452E 204E 6F20 5375 7070 6F72 7420"            /* ¨ÂE. No Support  */
	$"5365 7276 6963 6573 2E20 414E 5343 4120"            /* Services. ANSCA  */
	$"6973 2075 6E64 6572 206E 6F20 6F62 6C69"            /* is under no obli */
	$"6761 7469 6F6E 2074 6F20 7375 7070 6F72"            /* gation to suppor */
	$"7420 7468 6520 4465 7665 6C6F 7065 7220"            /* t the Developer  */
	$"536F 6674 7761 7265 2069 6E20 616E 7920"            /* Software in any  */
	$"7761 7920 6F72 2074 6F20 7072 6F76 6964"            /* way or to provid */
	$"6520 616E 7920 7570 6772 6164 6573 2074"            /* e any upgrades t */
	$"6F20 596F 752E 0A0A 462E 2052 6573 7472"            /* o You.ÂÂF. Restr */
	$"6963 7469 6F6E 732E 2059 6F75 2073 6861"            /* ictions. You sha */
	$"6C6C 206E 6F74 2063 6F70 7920 6F72 2075"            /* ll not copy or u */
	$"7365 2074 6865 2044 6576 656C 6F70 6572"            /* se the Developer */
	$"2053 6F66 7477 6172 6520 6578 6365 7074"            /*  Software except */
	$"2061 7320 6578 7072 6573 736C 7920 7065"            /*  as expressly pe */
	$"726D 6974 7465 6420 696E 2074 6869 7320"            /* rmitted in this  */
	$"4C69 6365 6E73 652E 2059 6F75 2077 696C"            /* License. You wil */
	$"6C20 6E6F 742C 2061 6E64 2077 696C 6C20"            /* l not, and will  */
	$"6E6F 7420 7065 726D 6974 2061 6E79 2074"            /* not permit any t */
	$"6869 7264 2070 6172 7479 2074 6F2C 2073"            /* hird party to, s */
	$"7562 6C69 6365 6E73 652C 2072 656E 742C"            /* ublicense, rent, */
	$"2063 6F70 792C 206D 6F64 6966 792C 2063"            /*  copy, modify, c */
	$"7265 6174 6520 6465 7269 7661 7469 7665"            /* reate derivative */
	$"2077 6F72 6B73 206F 662C 2074 7261 6E73"            /*  works of, trans */
	$"6C61 7465 2C20 7265 7665 7273 6520 656E"            /* late, reverse en */
	$"6769 6E65 6572 2C20 6465 636F 6D70 696C"            /* gineer, decompil */
	$"652C 2064 6973 6173 7365 6D62 6C65 2C20"            /* e, disassemble,  */
	$"6F72 206F 7468 6572 7769 7365 2072 6564"            /* or otherwise red */
	$"7563 6520 746F 2068 756D 616E 2070 6572"            /* uce to human per */
	$"6365 6976 6162 6C65 2066 6F72 6D20 616E"            /* ceivable form an */
	$"7920 706F 7274 696F 6E20 6F66 2074 6865"            /* y portion of the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652E 2054 6865 2044 6576 656C 6F70"            /* are. The Develop */
	$"6572 2053 6F66 7477 6172 6520 616E 6420"            /* er Software and  */
	$"616C 6C20 7065 7266 6F72 6D61 6E63 6520"            /* all performance  */
	$"6461 7461 2061 6E64 2074 6573 7420 7265"            /* data and test re */
	$"7375 6C74 732C 2069 6E63 6C75 6469 6E67"            /* sults, including */
	$"2077 6974 686F 7574 206C 696D 6974 6174"            /*  without limitat */
	$"696F 6E2C 2062 656E 6368 6D61 726B 2074"            /* ion, benchmark t */
	$"6573 7420 7265 7375 6C74 7320 2863 6F6C"            /* est results (col */
	$"6C65 6374 6976 656C 7920 2250 6572 666F"            /* lectively "Perfo */
	$"726D 616E 6365 2044 6174 6122 292C 2072"            /* rmance Data"), r */
	$"656C 6174 696E 6720 746F 2074 6865 2044"            /* elating to the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6172 6520 7468 6520 436F 6E66 6964"            /* e are the Confid */
	$"656E 7469 616C 2049 6E66 6F72 6D61 7469"            /* ential Informati */
	$"6F6E 206F 6620 414E 5343 412C 2061 6E64"            /* on of ANSCA, and */
	$"2077 696C 6C20 6265 2074 7265 6174 6564"            /*  will be treated */
	$"2069 6E20 6163 636F 7264 616E 6365 2077"            /*  in accordance w */
	$"6974 6820 7468 6520 7465 726D 7320 6F66"            /* ith the terms of */
	$"2073 6563 7469 6F6E 2035 206F 7220 756E"            /*  section 5 or un */
	$"6465 7220 616E 7920 6F62 6C69 6761 7469"            /* der any obligati */
	$"6F6E 206F 6620 636F 6E66 6964 656E 7469"            /* on of confidenti */
	$"616C 6974 792C 2077 6869 6368 6576 6572"            /* ality, whichever */
	$"2063 616D 6520 6669 7273 742E 2041 6363"            /*  came first. Acc */
	$"6F72 6469 6E67 6C79 2C20 596F 7520 7368"            /* ordingly, You sh */
	$"616C 6C20 6E6F 7420 7075 626C 6973 6820"            /* all not publish  */
	$"6F72 2064 6973 636C 6F73 6520 746F 2061"            /* or disclose to a */
	$"6E79 2074 6869 7264 2070 6172 7479 2061"            /* ny third party a */
	$"6E79 2050 6572 666F 726D 616E 6365 2044"            /* ny Performance D */
	$"6174 6120 7265 6C61 7469 6E67 2074 6F20"            /* ata relating to  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2E0A 0A47 2E20 4E6F 204F"            /* ftware.ÂÂG. No O */
	$"7468 6572 2050 6572 6D69 7474 6564 2055"            /* ther Permitted U */
	$"7365 732E 2045 7863 6570 7420 6173 206F"            /* ses. Except as o */
	$"7468 6572 7769 7365 2073 6574 2066 6F72"            /* therwise set for */
	$"7468 2069 6E20 7468 6973 2041 6772 6565"            /* th in this Agree */
	$"6D65 6E74 2C20 596F 7520 6167 7265 6520"            /* ment, You agree  */
	$"6E6F 7420 746F 2072 656E 742C 206C 6561"            /* not to rent, lea */
	$"7365 2C20 6C65 6E64 2C20 7570 6C6F 6164"            /* se, lend, upload */
	$"2074 6F20 6F72 2068 6F73 7420 6F6E 2061"            /*  to or host on a */
	$"6E79 2077 6562 7369 7465 206F 7220 7365"            /* ny website or se */
	$"7276 6572 2C20 7365 6C6C 2C20 7265 6469"            /* rver, sell, redi */
	$"7374 7269 6275 7465 2C20 6F72 2073 7562"            /* stribute, or sub */
	$"6C69 6365 6E73 6520 7468 6520 4465 7665"            /* license the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2C20"            /* loper Software,  */
	$"696E 2077 686F 6C65 206F 7220 696E 2070"            /* in whole or in p */
	$"6172 742C 206F 7220 746F 2065 6E61 626C"            /* art, or to enabl */
	$"6520 6F74 6865 7273 2074 6F20 646F 2073"            /* e others to do s */
	$"6F2E 2059 6F75 206D 6179 206E 6F74 2075"            /* o. You may not u */
	$"7365 2074 6865 2044 6576 656C 6F70 6572"            /* se the Developer */
	$"2053 6F66 7477 6172 6520 666F 7220 616E"            /*  Software for an */
	$"7920 7075 7270 6F73 6520 6E6F 7420 6578"            /* y purpose not ex */
	$"7072 6573 736C 7920 7065 726D 6974 7465"            /* pressly permitte */
	$"6420 6279 2074 6869 7320 4167 7265 656D"            /* d by this Agreem */
	$"656E 742E 2059 6F75 2061 6772 6565 206E"            /* ent. You agree n */
	$"6F74 2074 6F20 696E 7374 616C 6C2C 2075"            /* ot to install, u */
	$"7365 206F 7220 7275 6E20 7468 6520 4465"            /* se or run the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2069 6E20 636F 6E6E 6563 7469 6F6E 2077"            /*  in connection w */
	$"6974 6820 6465 7669 6365 7320 6F74 6865"            /* ith devices othe */
	$"7220 7468 616E 2041 4E53 4341 2D73 7570"            /* r than ANSCA-sup */
	$"706F 7274 6564 2064 6576 6963 6573 2C20"            /* ported devices,  */
	$"6F72 2074 6F20 656E 6162 6C65 206F 7468"            /* or to enable oth */
	$"6572 7320 746F 2064 6F20 736F 2E20 596F"            /* ers to do so. Yo */
	$"7520 6D61 7920 6E6F 7420 616E 6420 596F"            /* u may not and Yo */
	$"7520 6167 7265 6520 6E6F 7420 746F 2C20"            /* u agree not to,  */
	$"6F72 2074 6F20 656E 6162 6C65 206F 7468"            /* or to enable oth */
	$"6572 7320 746F 2C20 636F 7079 2028 6578"            /* ers to, copy (ex */
	$"6365 7074 2061 7320 6578 7072 6573 736C"            /* cept as expressl */
	$"7920 7065 726D 6974 7465 6420 756E 6465"            /* y permitted unde */
	$"7220 7468 6973 2041 6772 6565 6D65 6E74"            /* r this Agreement */
	$"292C 2064 6563 6F6D 7069 6C65 2C20 7265"            /* ), decompile, re */
	$"7665 7273 6520 656E 6769 6E65 6572 2C20"            /* verse engineer,  */
	$"6469 7361 7373 656D 626C 652C 2061 7474"            /* disassemble, att */
	$"656D 7074 2074 6F20 6465 7269 7665 2074"            /* empt to derive t */
	$"6865 2073 6F75 7263 6520 636F 6465 206F"            /* he source code o */
	$"662C 206D 6F64 6966 792C 2064 6563 7279"            /* f, modify, decry */
	$"7074 2C20 6F72 2063 7265 6174 6520 6465"            /* pt, or create de */
	$"7269 7661 7469 7665 2077 6F72 6B73 206F"            /* rivative works o */
	$"6620 7468 6520 4465 7665 6C6F 7065 7220"            /* f the Developer  */
	$"536F 6674 7761 7265 206F 7220 616E 7920"            /* Software or any  */
	$"7365 7276 6963 6573 2070 726F 7669 6465"            /* services provide */
	$"6420 6279 2074 6865 2044 6576 656C 6F70"            /* d by the Develop */
	$"6572 2053 6F66 7477 6172 652C 206F 7220"            /* er Software, or  */
	$"616E 7920 7061 7274 2074 6865 7265 6F66"            /* any part thereof */
	$"2028 6578 6365 7074 2061 7320 616E 6420"            /*  (except as and  */
	$"6F6E 6C79 2074 6F20 7468 6520 6578 7465"            /* only to the exte */
	$"6E74 2061 6E79 2066 6F72 6567 6F69 6E67"            /* nt any foregoing */
	$"2072 6573 7472 6963 7469 6F6E 2069 7320"            /*  restriction is  */
	$"7072 6F68 6962 6974 6564 2062 7920 6170"            /* prohibited by ap */
	$"706C 6963 6162 6C65 206C 6177 206F 7220"            /* plicable law or  */
	$"746F 2074 6865 2065 7874 656E 7420 6173"            /* to the extent as */
	$"206D 6179 2062 6520 7065 726D 6974 7465"            /*  may be permitte */
	$"6420 6279 206C 6963 656E 7369 6E67 2074"            /* d by licensing t */
	$"6572 6D73 2067 6F76 6572 6E69 6E67 2075"            /* erms governing u */
	$"7365 206F 6620 4F70 656E 2053 6F75 7263"            /* se of Open Sourc */
	$"6564 2043 6F6D 706F 6E65 6E74 7320 6F72"            /* ed Components or */
	$"2053 616D 706C 6520 436F 6465 2069 6E63"            /*  Sample Code inc */
	$"6C75 6465 6420 7769 7468 2074 6865 2053"            /* luded with the S */
	$"444B 292E 2059 6F75 2061 6772 6565 206E"            /* DK). You agree n */
	$"6F74 2074 6F20 6578 706C 6F69 7420 616E"            /* ot to exploit an */
	$"7920 7365 7276 6963 6573 2070 726F 7669"            /* y services provi */
	$"6465 6420 6279 2074 6865 2053 444B 2069"            /* ded by the SDK i */
	$"6E20 616E 7920 756E 6175 7468 6F72 697A"            /* n any unauthoriz */
	$"6564 2077 6179 2077 6861 7473 6F65 7665"            /* ed way whatsoeve */
	$"722C 2069 6E63 6C75 6469 6E67 2062 7574"            /* r, including but */
	$"206E 6F74 206C 696D 6974 6564 2074 6F2C"            /*  not limited to, */
	$"2062 7920 7472 6573 7061 7373 206F 7220"            /*  by trespass or  */
	$"6275 7264 656E 696E 6720 6E65 7477 6F72"            /* burdening networ */
	$"6B20 6361 7061 6369 7479 2E20 416E 7920"            /* k capacity. Any  */
	$"6174 7465 6D70 7420 746F 2064 6F20 736F"            /* attempt to do so */
	$"2069 7320 6120 7669 6F6C 6174 696F 6E20"            /*  is a violation  */
	$"6F66 2074 6865 2072 6967 6874 7320 6F66"            /* of the rights of */
	$"2041 4E53 4341 2061 6E64 2069 7473 206C"            /*  ANSCA and its l */
	$"6963 656E 736F 7273 206F 6620 7468 6520"            /* icensors of the  */
	$"4465 7665 6C6F 7065 7220 536F 6674 7761"            /* Developer Softwa */
	$"7265 206F 7220 7365 7276 6963 6573 2070"            /* re or services p */
	$"726F 7669 6465 6420 6279 2074 6865 2044"            /* rovided by the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"652E 2049 6620 596F 7520 6272 6561 6368"            /* e. If You breach */
	$"2061 6E79 206F 6620 7468 6520 666F 7265"            /*  any of the fore */
	$"676F 696E 6720 7265 7374 7269 6374 696F"            /* going restrictio */
	$"6E73 2C20 596F 7520 6D61 7920 6265 2073"            /* ns, You may be s */
	$"7562 6A65 6374 2074 6F20 7072 6F73 6563"            /* ubject to prosec */
	$"7574 696F 6E20 616E 6420 6461 6D61 6765"            /* ution and damage */
	$"732E 2041 6C6C 206C 6963 656E 7365 7320"            /* s. All licenses  */
	$"6E6F 7420 6578 7072 6573 736C 7920 6772"            /* not expressly gr */
	$"616E 7465 6420 696E 2074 6869 7320 4167"            /* anted in this Ag */
	$"7265 656D 656E 7420 6172 6520 7265 7365"            /* reement are rese */
	$"7276 6564 2061 6E64 206E 6F20 6F74 6865"            /* rved and no othe */
	$"7220 6C69 6365 6E73 6573 2C20 696D 6D75"            /* r licenses, immu */
	$"6E69 7479 206F 7220 7269 6768 7473 2C20"            /* nity or rights,  */
	$"6578 7072 6573 7320 6F72 2069 6D70 6C69"            /* express or impli */
	$"6564 2061 7265 2067 7261 6E74 6564 2062"            /* ed are granted b */
	$"7920 414E 5343 412C 2062 7920 696D 706C"            /* y ANSCA, by impl */
	$"6963 6174 696F 6E2C 2065 7374 6F70 7065"            /* ication, estoppe */
	$"6C2C 206F 7220 6F74 6865 7277 6973 652E"            /* l, or otherwise. */
	$"2054 6869 7320 4167 7265 656D 656E 7420"            /*  This Agreement  */
	$"646F 6573 206E 6F74 2067 7261 6E74 2059"            /* does not grant Y */
	$"6F75 2061 6E79 2072 6967 6874 7320 746F"            /* ou any rights to */
	$"2075 7365 2061 6E79 2074 7261 6465 6D61"            /*  use any tradema */
	$"726B 732C 206C 6F67 6F73 206F 7220 7365"            /* rks, logos or se */
	$"7276 6963 6520 6D61 726B 7320 6265 6C6F"            /* rvice marks belo */
	$"6E67 696E 6720 746F 2041 4E53 4341 2E20"            /* nging to ANSCA.  */
	$"0A0A 482E 204F 7468 6572 2052 6573 7472"            /* ÂÂH. Other Restr */
	$"6963 7469 6F6E 732E 2020 4578 6365 7074"            /* ictions.  Except */
	$"2061 7320 616E 6420 6F6E 6C79 2074 6F20"            /*  as and only to  */
	$"7468 6520 6578 7465 6E74 2065 7870 7265"            /* the extent expre */
	$"7373 6C79 2070 6572 6D69 7474 6564 2062"            /* ssly permitted b */
	$"7920 7468 6973 204C 6963 656E 7365 2C20"            /* y this License,  */
	$"6279 2061 7070 6C69 6361 626C 6520 6C69"            /* by applicable li */
	$"6365 6E73 696E 6720 7465 726D 7320 676F"            /* censing terms go */
	$"7665 726E 696E 6720 7573 6520 6F66 2074"            /* verning use of t */
	$"6865 204F 7065 6E20 536F 7572 6365 6420"            /* he Open Sourced  */
	$"436F 6D70 6F6E 656E 7473 206F 7220 5361"            /* Components or Sa */
	$"6D70 6C65 2043 6F64 652C 206F 7220 746F"            /* mple Code, or to */
	$"2074 6865 2065 7874 656E 7420 7468 6174"            /*  the extent that */
	$"2074 6865 2066 6F6C 6C6F 7769 6E67 2072"            /*  the following r */
	$"6573 7472 6963 7469 6F6E 7320 6172 6520"            /* estrictions are  */
	$"7072 6F68 6962 6974 6564 2062 7920 6170"            /* prohibited by ap */
	$"706C 6963 6162 6C65 206C 6177 2C20 596F"            /* plicable law, Yo */
	$"7520 6D61 7920 6E6F 7420 636F 7079 2C20"            /* u may not copy,  */
	$"6465 636F 6D70 696C 652C 2072 6576 6572"            /* decompile, rever */
	$"7365 2065 6E67 696E 6565 722C 2064 6973"            /* se engineer, dis */
	$"6173 7365 6D62 6C65 2C20 6174 7465 6D70"            /* assemble, attemp */
	$"7420 746F 2064 6572 6976 6520 7468 6520"            /* t to derive the  */
	$"736F 7572 6365 2063 6F64 6520 6F66 2074"            /* source code of t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 652C 206D 6F64 6966 792C 2064"            /* tware, modify, d */
	$"6563 7279 7074 2C20 6372 6561 7465 2064"            /* ecrypt, create d */
	$"6572 6976 6174 6976 6520 776F 726B 7320"            /* erivative works  */
	$"6F66 2C20 696E 636F 7270 6F72 6174 6520"            /* of, incorporate  */
	$"696E 746F 206F 7220 636F 6D70 696C 6520"            /* into or compile  */
	$"696E 2063 6F6D 6269 6E61 7469 6F6E 2077"            /* in combination w */
	$"6974 6820 596F 7572 206F 776E 2070 726F"            /* ith Your own pro */
	$"6772 616D 732C 2073 7562 6C69 6365 6E73"            /* grams, sublicens */
	$"6520 6F72 206F 7468 6572 7769 7365 2072"            /* e or otherwise r */
	$"6564 6973 7472 6962 7574 6520 7468 6520"            /* edistribute the  */
	$"4465 7665 6C6F 7065 7220 536F 6674 7761"            /* Developer Softwa */
	$"7265 2E20 0A0A 332E 2054 7261 6E73 6665"            /* re. ÂÂ3. Transfe */
	$"722E 2045 7863 6570 7420 6173 206F 7468"            /* r. Except as oth */
	$"6572 7769 7365 2073 6574 2066 6F72 7468"            /* erwise set forth */
	$"2069 6E20 7468 6973 2041 6772 6565 6D65"            /*  in this Agreeme */
	$"6E74 2C20 596F 7520 6D61 7920 6E6F 7420"            /* nt, You may not  */
	$"7265 6E74 2C20 6C65 6173 652C 206C 656E"            /* rent, lease, len */
	$"642C 2072 6564 6973 7472 6962 7574 6520"            /* d, redistribute  */
	$"6F72 2073 7562 6C69 6365 6E73 6520 7468"            /* or sublicense th */
	$"6520 4465 7665 6C6F 7065 7220 536F 6674"            /* e Developer Soft */
	$"7761 7265 2E20 2059 6F75 206D 6179 2C20"            /* ware.  You may,  */
	$"686F 7765 7665 722C 206D 616B 6520 6120"            /* however, make a  */
	$"6F6E 652D 7469 6D65 2070 6572 6D61 6E65"            /* one-time permane */
	$"6E74 2074 7261 6E73 6665 7220 6F66 2061"            /* nt transfer of a */
	$"6C6C 206F 6620 596F 7572 206C 6963 656E"            /* ll of Your licen */
	$"7365 2072 6967 6874 7320 746F 2074 6865"            /* se rights to the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 6520 2869 6E20 6974 7320 6F72 6967"            /* are (in its orig */
	$"696E 616C 2066 6F72 6D20 6173 2070 726F"            /* inal form as pro */
	$"7669 6465 6420 6279 2041 4E53 4341 2920"            /* vided by ANSCA)  */
	$"746F 2061 6E6F 7468 6572 2070 6172 7479"            /* to another party */
	$"2C20 7072 6F76 6964 6564 2074 6861 743A"            /* , provided that: */
	$"2028 6129 2074 6865 2074 7261 6E73 6665"            /*  (a) the transfe */
	$"7220 6D75 7374 2069 6E63 6C75 6465 2061"            /* r must include a */
	$"6C6C 206F 6620 7468 6520 4465 7665 6C6F"            /* ll of the Develo */
	$"7065 7220 536F 6674 7761 7265 2C20 696E"            /* per Software, in */
	$"636C 7564 696E 6720 616C 6C20 6974 7320"            /* cluding all its  */
	$"636F 6D70 6F6E 656E 7420 7061 7274 732C"            /* component parts, */
	$"206F 7269 6769 6E61 6C20 6D65 6469 612C"            /*  original media, */
	$"2070 7269 6E74 6564 206D 6174 6572 6961"            /*  printed materia */
	$"6C73 2061 6E64 2074 6869 7320 4C69 6365"            /* ls and this Lice */
	$"6E73 653B 2028 6229 2059 6F75 2064 6F20"            /* nse; (b) You do  */
	$"6E6F 7420 7265 7461 696E 2061 6E79 2063"            /* not retain any c */
	$"6F70 6965 7320 6F66 2074 6865 2044 6576"            /* opies of the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 652C"            /* eloper Software, */
	$"2066 756C 6C20 6F72 2070 6172 7469 616C"            /*  full or partial */
	$"2C20 696E 636C 7564 696E 6720 636F 7069"            /* , including copi */
	$"6573 2073 746F 7265 6420 6F6E 2061 2063"            /* es stored on a c */
	$"6F6D 7075 7465 7220 6F72 206F 7468 6572"            /* omputer or other */
	$"2073 746F 7261 6765 2064 6576 6963 653B"            /*  storage device; */
	$"2061 6E64 2028 6329 2074 6865 2070 6172"            /*  and (c) the par */
	$"7479 2072 6563 6569 7669 6E67 2074 6865"            /* ty receiving the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 6520 7265 6164 7320 616E 6420 6167"            /* are reads and ag */
	$"7265 6573 2074 6F20 6163 6365 7074 2074"            /* rees to accept t */
	$"6865 2074 6572 6D73 2061 6E64 2063 6F6E"            /* he terms and con */
	$"6469 7469 6F6E 7320 6F66 2074 6869 7320"            /* ditions of this  */
	$"4C69 6365 6E73 652E 2059 6F75 206D 6179"            /* License. You may */
	$"206E 6F74 2072 656E 742C 206C 6561 7365"            /*  not rent, lease */
	$"2C20 6C65 6E64 2C20 7265 6469 7374 7269"            /* , lend, redistri */
	$"6275 7465 2C20 7375 626C 6963 656E 7365"            /* bute, sublicense */
	$"206F 7220 7472 616E 7366 6572 2061 6E79"            /*  or transfer any */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 6520 7468 6174 2068 6173 2062 6565"            /* are that has bee */
	$"6E20 6D6F 6469 6669 6564 206F 7220 7265"            /* n modified or re */
	$"706C 6163 6564 2075 6E64 6572 2053 6563"            /* placed under Sec */
	$"7469 6F6E 2032 2862 2920 6162 6F76 652E"            /* tion 2(b) above. */
	$"2041 6C6C 2063 6F6D 706F 6E65 6E74 7320"            /*  All components  */
	$"6F66 2074 6865 2044 6576 656C 6F70 6572"            /* of the Developer */
	$"2053 6F66 7477 6172 6520 6172 6520 7072"            /*  Software are pr */
	$"6F76 6964 6564 2061 7320 7061 7274 206F"            /* ovided as part o */
	$"6620 6120 6275 6E64 6C65 2061 6E64 206D"            /* f a bundle and m */
	$"6179 206E 6F74 2062 6520 7365 7061 7261"            /* ay not be separa */
	$"7465 6420 6672 6F6D 2074 6865 2062 756E"            /* ted from the bun */
	$"646C 6520 616E 6420 6469 7374 7269 6275"            /* dle and distribu */
	$"7465 6420 6173 2073 7461 6E64 616C 6F6E"            /* ted as standalon */
	$"6520 6170 706C 6963 6174 696F 6E73 2E20"            /* e applications.  */
	$"200A 0A4E 4652 2028 4E6F 7420 666F 7220"            /*  ÂÂNFR (Not for  */
	$"5265 7361 6C65 2920 616E 6420 4576 616C"            /* Resale) and Eval */
	$"7561 7469 6F6E 2043 6F70 6965 733A 204E"            /* uation Copies: N */
	$"6F74 7769 7468 7374 616E 6469 6E67 206F"            /* otwithstanding o */
	$"7468 6572 2073 6563 7469 6F6E 7320 6F66"            /* ther sections of */
	$"2074 6869 7320 4C69 6365 6E73 652C 2044"            /*  this License, D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6C61 6265 6C65 6420 6F72 206F 7468"            /* e labeled or oth */
	$"6572 7769 7365 2070 726F 7669 6465 6420"            /* erwise provided  */
	$"746F 2059 6F75 206F 6E20 6120 7072 6F6D"            /* to You on a prom */
	$"6F74 696F 6E61 6C20 6261 7369 7320 6D61"            /* otional basis ma */
	$"7920 6F6E 6C79 2062 6520 7573 6564 2066"            /* y only be used f */
	$"6F72 2064 656D 6F6E 7374 7261 7469 6F6E"            /* or demonstration */
	$"2C20 7465 7374 696E 6720 616E 6420 6576"            /* , testing and ev */
	$"616C 7561 7469 6F6E 2070 7572 706F 7365"            /* aluation purpose */
	$"7320 616E 6420 6D61 7920 6E6F 7420 6265"            /* s and may not be */
	$"2072 6573 6F6C 6420 6F72 2074 7261 6E73"            /*  resold or trans */
	$"6665 7272 6564 2E20 0A0A 342E 2054 6572"            /* ferred. ÂÂ4. Ter */
	$"6D20 616E 6420 5465 726D 696E 6174 696F"            /* m and Terminatio */
	$"6E2E 200A 0A41 2E20 5465 726D 2E20 596F"            /* n. ÂÂA. Term. Yo */
	$"7572 2072 6967 6874 7320 7769 7468 2072"            /* ur rights with r */
	$"6573 7065 6374 2074 6F20 7468 6520 4465"            /* espect to the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2077 696C 6C20 7465 726D 696E 6174 6520"            /*  will terminate  */
	$"7570 6F6E 2074 6865 2065 6172 6C69 6572"            /* upon the earlier */
	$"206F 6620 2861 2920 7468 6520 696E 6974"            /*  of (a) the init */
	$"6961 6C20 636F 6D6D 6572 6369 616C 2072"            /* ial commercial r */
	$"656C 6561 7365 2062 7920 414E 5343 4120"            /* elease by ANSCA  */
	$"6F66 2061 2067 656E 6572 616C 6C79 2061"            /* of a generally a */
	$"7661 696C 6162 6C65 2076 6572 7369 6F6E"            /* vailable version */
	$"206F 6620 7468 6520 4465 7665 6C6F 7065"            /*  of the Develope */
	$"7220 536F 6674 7761 7265 206F 7220 2862"            /* r Software or (b */
	$"2920 6175 746F 6D61 7469 6320 6578 7069"            /* ) automatic expi */
	$"7261 7469 6F6E 206F 6620 7468 6520 4465"            /* ration of the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2062 6173 6564 206F 6E20 7468 6520 7379"            /*  based on the sy */
	$"7374 656D 2064 6174 652E 2045 6974 6865"            /* stem date. Eithe */
	$"7220 7061 7274 7920 6D61 7920 7465 726D"            /* r party may term */
	$"696E 6174 6520 7468 6973 204C 6963 656E"            /* inate this Licen */
	$"7365 2061 7420 616E 7920 7469 6D65 2066"            /* se at any time f */
	$"6F72 2061 6E79 2072 6561 736F 6E20 6F72"            /* or any reason or */
	$"206E 6F20 7265 6173 6F6E 2062 7920 7072"            /*  no reason by pr */
	$"6F76 6964 696E 6720 7468 6520 6F74 6865"            /* oviding the othe */
	$"7220 7061 7274 7920 6164 7661 6E63 6520"            /* r party advance  */
	$"7772 6974 7465 6E20 6E6F 7469 6365 2074"            /* written notice t */
	$"6865 7265 6F66 2E20 5570 6F6E 2061 6E79"            /* hereof. Upon any */
	$"2065 7870 6972 6174 696F 6E20 6F72 2074"            /*  expiration or t */
	$"6572 6D69 6E61 7469 6F6E 206F 6620 7468"            /* ermination of th */
	$"6973 204C 6963 656E 7365 2C20 7468 6520"            /* is License, the  */
	$"7269 6768 7473 2061 6E64 206C 6963 656E"            /* rights and licen */
	$"7365 7320 6772 616E 7465 6420 746F 2059"            /* ses granted to Y */
	$"6F75 2075 6E64 6572 2074 6869 7320 4C69"            /* ou under this Li */
	$"6365 6E73 6520 7368 616C 6C20 696D 6D65"            /* cense shall imme */
	$"6469 6174 656C 7920 7465 726D 696E 6174"            /* diately terminat */
	$"652C 2061 6E64 2059 6F75 2073 6861 6C6C"            /* e, and You shall */
	$"2069 6D6D 6564 6961 7465 6C79 2063 6561"            /*  immediately cea */
	$"7365 2075 7369 6E67 2C20 616E 6420 7769"            /* se using, and wi */
	$"6C6C 2072 6574 7572 6E20 746F 2041 4E53"            /* ll return to ANS */
	$"4341 2028 6F72 2C20 6174 2041 4E53 4341"            /* CA (or, at ANSCA */
	$"2773 2072 6571 7565 7374 2C20 6465 7374"            /* 's request, dest */
	$"726F 7929 2C20 7468 6520 4465 7665 6C6F"            /* roy), the Develo */
	$"7065 7220 536F 6674 7761 7265 2074 6861"            /* per Software tha */
	$"7420 6172 6520 7072 6F70 7269 6574 6172"            /* t are proprietar */
	$"7920 746F 206F 7220 636F 6E74 6169 6E20"            /* y to or contain  */
	$"436F 6E66 6964 656E 7469 616C 2049 6E66"            /* Confidential Inf */
	$"6F72 6D61 7469 6F6E 2E20 5468 6520 7269"            /* ormation. The ri */
	$"6768 7473 2061 6E64 206F 626C 6967 6174"            /* ghts and obligat */
	$"696F 6E73 206F 6620 7468 6520 7061 7274"            /* ions of the part */
	$"6965 7320 7365 7420 666F 7274 6820 696E"            /* ies set forth in */
	$"2053 6563 7469 6F6E 7320 3228 6129 2869"            /*  Sections 2(a)(i */
	$"6969 292C 2032 2863 292C 2032 2864 292C"            /* ii), 2(c), 2(d), */
	$"2032 2864 292C 2032 2865 292C 2032 2866"            /*  2(d), 2(e), 2(f */
	$"292C 2032 2867 292C 2033 2C20 342C 2035"            /* ), 2(g), 3, 4, 5 */
	$"2C20 3620 616E 6420 3720 7368 616C 6C20"            /* , 6 and 7 shall  */
	$"7375 7276 6976 6520 7465 726D 696E 6174"            /* survive terminat */
	$"696F 6E20 6F72 2065 7870 6972 6174 696F"            /* ion or expiratio */
	$"6E20 6F66 2074 6869 7320 4C69 6365 6E73"            /* n of this Licens */
	$"6520 666F 7220 616E 7920 7265 6173 6F6E"            /* e for any reason */
	$"2E0A 0A42 2E20 5465 726D 696E 6174 696F"            /* .ÂÂB. Terminatio */
	$"6E2E 2054 6869 7320 4C69 6365 6E73 6520"            /* n. This License  */
	$"6973 2065 6666 6563 7469 7665 2075 6E74"            /* is effective unt */
	$"696C 2074 6572 6D69 6E61 7465 642E 2059"            /* il terminated. Y */
	$"6F75 7220 7269 6768 7473 2075 6E64 6572"            /* our rights under */
	$"2074 6869 7320 4C69 6365 6E73 6520 7769"            /*  this License wi */
	$"6C6C 2074 6572 6D69 6E61 7465 2061 7574"            /* ll terminate aut */
	$"6F6D 6174 6963 616C 6C79 2077 6974 686F"            /* omatically witho */
	$"7574 206E 6F74 6963 6520 6672 6F6D 2041"            /* ut notice from A */
	$"4E53 4341 2069 6620 596F 7520 6661 696C"            /* NSCA if You fail */
	$"2074 6F20 636F 6D70 6C79 2077 6974 6820"            /*  to comply with  */
	$"616E 7920 7465 726D 2873 2920 6F66 2074"            /* any term(s) of t */
	$"6869 7320 4C69 6365 6E73 652E 2049 6E20"            /* his License. In  */
	$"6164 6469 7469 6F6E 2C20 414E 5343 4120"            /* addition, ANSCA  */
	$"7265 7365 7276 6573 2074 6865 2072 6967"            /* reserves the rig */
	$"6874 2074 6F20 7465 726D 696E 6174 6520"            /* ht to terminate  */
	$"7468 6973 204C 6963 656E 7365 2069 6620"            /* this License if  */
	$"6120 6E65 7720 7665 7273 696F 6E20 6F66"            /* a new version of */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 6520 6973 2072 656C 6561"            /* oftware is relea */
	$"7365 6420 7768 6963 6820 6973 2069 6E63"            /* sed which is inc */
	$"6F6D 7061 7469 626C 6520 7769 7468 2074"            /* ompatible with t */
	$"6869 7320 7665 7273 696F 6E20 6F66 2074"            /* his version of t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 652E 2055 706F 6E20 7468 6520"            /* tware. Upon the  */
	$"7465 726D 696E 6174 696F 6E20 6F66 2074"            /* termination of t */
	$"6869 7320 4C69 6365 6E73 652C 2059 6F75"            /* his License, You */
	$"2073 6861 6C6C 2063 6561 7365 2061 6C6C"            /*  shall cease all */
	$"2075 7365 206F 6620 7468 6520 4465 7665"            /*  use of the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2061"            /* loper Software a */
	$"6E64 2064 6573 7472 6F79 2061 6C6C 2063"            /* nd destroy all c */
	$"6F70 6965 732C 2066 756C 6C20 6F72 2070"            /* opies, full or p */
	$"6172 7469 616C 2C20 6F66 2074 6865 2044"            /* artial, of the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"652E 0A0A 432E 2043 6861 6E67 6573 2074"            /* e.ÂÂC. Changes t */
	$"6F20 5465 726D 732E 2041 4E53 4341 206D"            /* o Terms. ANSCA m */
	$"6179 2063 6861 6E67 6520 7468 6520 7465"            /* ay change the te */
	$"726D 7320 616E 6420 636F 6E64 6974 696F"            /* rms and conditio */
	$"6E73 206F 6620 7468 6973 204C 6963 656E"            /* ns of this Licen */
	$"7365 2061 7420 616E 7920 7469 6D65 2E20"            /* se at any time.  */
	$"4E65 7720 6F72 206D 6F64 6966 6965 6420"            /* New or modified  */
	$"7465 726D 7320 616E 6420 636F 6E64 6974"            /* terms and condit */
	$"696F 6E73 2077 696C 6C20 6E6F 7420 7265"            /* ions will not re */
	$"7472 6F61 6374 6976 656C 7920 6170 706C"            /* troactively appl */
	$"7920 746F 2041 7070 6C69 6361 7469 6F6E"            /* y to Application */
	$"7320 6372 6561 7465 6420 7769 7468 2074"            /* s created with t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 6520 616C 7265 6164 7920 696E"            /* tware already in */
	$"2064 6973 7472 6962 7574 696F 6E2E 2049"            /*  distribution. I */
	$"6E20 6F72 6465 7220 746F 2063 6F6E 7469"            /* n order to conti */
	$"6E75 6520 7573 696E 6720 7468 6520 4465"            /* nue using the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2C20 596F 7520 6D75 7374 2061 6363 6570"            /* , You must accep */
	$"7420 616E 6420 6167 7265 6520 746F 2074"            /* t and agree to t */
	$"6865 206E 6577 2074 6572 6D73 2061 6E64"            /* he new terms and */
	$"2063 6F6E 6469 7469 6F6E 7320 6F66 2074"            /*  conditions of t */
	$"6869 7320 4C69 6365 6E73 652E 2049 6620"            /* his License. If  */
	$"596F 7520 646F 206E 6F74 2061 6772 6565"            /* You do not agree */
	$"2074 6F20 7468 6520 6E65 7720 7465 726D"            /*  to the new term */
	$"7320 616E 6420 636F 6E64 6974 696F 6E73"            /* s and conditions */
	$"2C20 596F 7572 2075 7365 206F 6620 7468"            /* , Your use of th */
	$"6520 4465 7665 6C6F 7065 7220 536F 6674"            /* e Developer Soft */
	$"7761 7265 2077 696C 6C20 6265 2073 7573"            /* ware will be sus */
	$"7065 6E64 6564 206F 7220 7465 726D 696E"            /* pended or termin */
	$"6174 6564 2062 7920 414E 5343 412E 2059"            /* ated by ANSCA. Y */
	$"6F75 2061 6772 6565 2074 6861 7420 596F"            /* ou agree that Yo */
	$"7572 2061 6363 6570 7461 6E63 6520 6F66"            /* ur acceptance of */
	$"2073 7563 6820 6E65 7720 4C69 6365 6E73"            /*  such new Licens */
	$"6520 7465 726D 7320 616E 6420 636F 6E64"            /* e terms and cond */
	$"6974 696F 6E73 206D 6179 2062 6520 7369"            /* itions may be si */
	$"676E 6966 6965 6420 656C 6563 7472 6F6E"            /* gnified electron */
	$"6963 616C 6C79 2C20 696E 636C 7564 696E"            /* ically, includin */
	$"6720 7769 7468 6F75 7420 6C69 6D69 7461"            /* g without limita */
	$"7469 6F6E 2C20 6279 2059 6F75 7220 6368"            /* tion, by Your ch */
	$"6563 6B69 6E67 2061 2062 6F78 206F 7220"            /* ecking a box or  */
	$"636C 6963 6B69 6E67 206F 6E20 616E 2022"            /* clicking on an " */
	$"6167 7265 6522 206F 7220 7369 6D69 6C61"            /* agree" or simila */
	$"7220 6275 7474 6F6E 2E20 4E6F 7468 696E"            /* r button. Nothin */
	$"6720 696E 2074 6869 7320 5365 6374 696F"            /* g in this Sectio */
	$"6E20 7368 616C 6C20 6166 6665 6374 2041"            /* n shall affect A */
	$"4E53 4341 2773 2072 6967 6874 7320 756E"            /* NSCA's rights un */
	$"6465 7220 5365 6374 696F 6E20 3428 6429"            /* der Section 4(d) */
	$"2062 656C 6F77 2E0A 0A44 2E20 5265 766F"            /*  below.ÂÂD. Revo */
	$"6361 7469 6F6E 2E20 596F 7520 756E 6465"            /* cation. You unde */
	$"7273 7461 6E64 2061 6E64 2061 6772 6565"            /* rstand and agree */
	$"2074 6861 7420 414E 5343 4120 6D61 7920"            /*  that ANSCA may  */
	$"6365 6173 6520 596F 7572 2061 6269 6C69"            /* cease Your abili */
	$"7479 2074 6F20 6372 6561 7465 2041 7070"            /* ty to create App */
	$"6C69 6361 7469 6F6E 7320 6F72 206F 7468"            /* lications or oth */
	$"6572 2073 6F66 7477 6172 6520 7573 696E"            /* er software usin */
	$"6720 7468 6520 5344 4B20 6174 2061 6E79"            /* g the SDK at any */
	$"2074 696D 652E 2042 7920 7761 7920 6F66"            /*  time. By way of */
	$"2065 7861 6D70 6C65 206F 6E6C 792C 2041"            /*  example only, A */
	$"4E53 4341 206D 6967 6874 2063 686F 6F73"            /* NSCA might choos */
	$"6520 746F 2064 6F20 7468 6973 2069 6620"            /* e to do this if  */
	$"6174 2061 6E79 2074 696D 653A 200A 2861"            /* at any time: Â(a */
	$"2920 414E 5343 4120 6861 7320 6265 656E"            /* ) ANSCA has been */
	$"206E 6F74 6966 6965 6420 6F72 206F 7468"            /*  notified or oth */
	$"6572 7769 7365 2068 6173 2072 6561 736F"            /* erwise has reaso */
	$"6E20 746F 2062 656C 6965 7665 2074 6861"            /* n to believe tha */
	$"7420 596F 7572 2041 7070 6C69 6361 7469"            /* t Your Applicati */
	$"6F6E 2076 696F 6C61 7465 732C 206D 6973"            /* on violates, mis */
	$"6170 7072 6F70 7269 6174 6573 2C20 6F72"            /* appropriates, or */
	$"2069 6E66 7269 6E67 6573 2074 6865 2072"            /*  infringes the r */
	$"6967 6874 7320 6F66 2061 2074 6869 7264"            /* ights of a third */
	$"2070 6172 7479 206F 7220 6F66 2041 4E53"            /*  party or of ANS */
	$"4341 3B20 0A28 6229 2041 4E53 4341 2068"            /* CA; Â(b) ANSCA h */
	$"6173 2072 6561 736F 6E20 746F 2062 656C"            /* as reason to bel */
	$"6965 7665 2074 6861 7420 596F 7520 6861"            /* ieve that You ha */
	$"7665 206F 7220 7769 6C6C 2063 7265 6174"            /* ve or will creat */
	$"6520 4170 706C 6963 6174 696F 6E73 2075"            /* e Applications u */
	$"7369 6E67 2074 6865 2053 444B 2074 6861"            /* sing the SDK tha */
	$"7420 636F 6E74 6169 6E20 6D61 6C69 6369"            /* t contain malici */
	$"6F75 7320 6F72 2068 6172 6D66 756C 2063"            /* ous or harmful c */
	$"6F64 652C 206D 616C 7761 7265 2C20 7072"            /* ode, malware, pr */
	$"6F67 7261 6D73 206F 7220 6F74 6865 7220"            /* ograms or other  */
	$"696E 7465 726E 616C 2063 6F6D 706F 6E65"            /* internal compone */
	$"6E74 7320 2865 2E67 2E20 736F 6674 7761"            /* nts (e.g. softwa */
	$"7265 2076 6972 7573 293B 200A 2863 2920"            /* re virus); Â(c)  */
	$"414E 5343 4120 6861 7320 7265 6173 6F6E"            /* ANSCA has reason */
	$"2074 6F20 6265 6C69 6576 6520 7468 6174"            /*  to believe that */
	$"2059 6F75 7220 6170 706C 6963 6174 696F"            /*  Your applicatio */
	$"6E20 6461 6D61 6765 732C 2063 6F72 7275"            /* n damages, corru */
	$"7074 732C 2064 6567 7261 6465 732C 2064"            /* pts, degrades, d */
	$"6573 7472 6F79 7320 6F72 206F 7468 6572"            /* estroys or other */
	$"7769 7365 2061 6476 6572 7365 6C79 2061"            /* wise adversely a */
	$"6666 6563 7473 2074 6865 2064 6576 6963"            /* ffects the devic */
	$"6573 2069 7420 6F70 6572 6174 6573 206F"            /* es it operates o */
	$"6E2C 206F 7220 616E 7920 0A6F 7468 6572"            /* n, or any Âother */
	$"2073 6F66 7477 6172 652C 2066 6972 6D77"            /*  software, firmw */
	$"6172 652C 2068 6172 6477 6172 652C 2064"            /* are, hardware, d */
	$"6174 612C 2073 7973 7465 6D73 2C20 6F72"            /* ata, systems, or */
	$"206E 6574 776F 726B 7320 6163 6365 7373"            /*  networks access */
	$"6564 206F 7220 7573 6564 2062 7920 7468"            /* ed or used by th */
	$"6520 6170 706C 6963 6174 696F 6E3B 200A"            /* e application; Â */
	$"2864 2920 596F 7520 6272 6561 6368 2061"            /* (d) You breach a */
	$"6E79 2074 6572 6D20 6F72 2063 6F6E 6469"            /* ny term or condi */
	$"7469 6F6E 206F 6620 7468 6973 204C 6963"            /* tion of this Lic */
	$"656E 7365 3B0A 2865 2920 416E 7920 696E"            /* ense;Â(e) Any in */
	$"666F 726D 6174 696F 6E20 6F72 2064 6F63"            /* formation or doc */
	$"756D 656E 7473 2070 726F 7669 6465 6420"            /* uments provided  */
	$"6279 2059 6F75 2074 6F20 414E 5343 4120"            /* by You to ANSCA  */
	$"666F 7220 7468 6520 7075 7270 6F73 6520"            /* for the purpose  */
	$"6F66 2076 6572 6966 7969 6E67 2059 6F75"            /* of verifying You */
	$"7220 6964 656E 7469 7479 206F 7220 6F62"            /* r identity or ob */
	$"7461 696E 696E 6720 414E 5343 412D 6973"            /* taining ANSCA-is */
	$"7375 6564 2064 6967 6974 616C 2063 6572"            /* sued digital cer */
	$"7469 6669 6361 7465 7320 6973 2066 616C"            /* tificates is fal */
	$"7365 206F 7220 696E 6163 6375 7261 7465"            /* se or inaccurate */
	$"3B20 0A28 6629 2041 6E79 2072 6570 7265"            /* ; Â(f) Any repre */
	$"7365 6E74 6174 696F 6E2C 2077 6172 7261"            /* sentation, warra */
	$"6E74 7920 6F72 2063 6572 7469 6669 6361"            /* nty or certifica */
	$"7469 6F6E 2070 726F 7669 6465 6420 6279"            /* tion provided by */
	$"2059 6F75 2074 6F20 414E 5343 4120 696E"            /*  You to ANSCA in */
	$"2074 6869 7320 4C69 6365 6E73 6520 6973"            /*  this License is */
	$"2075 6E74 7275 6520 6F72 2069 6E61 6363"            /*  untrue or inacc */
	$"7572 6174 653B 200A 2867 2920 414E 5343"            /* urate; Â(g) ANSC */
	$"4120 6973 2072 6571 7569 7265 6420 6279"            /* A is required by */
	$"206C 6177 2C20 7265 6775 6C61 7469 6F6E"            /*  law, regulation */
	$"206F 7220 6F74 6865 7220 676F 7665 726E"            /*  or other govern */
	$"6D65 6E74 616C 206F 7220 636F 7572 7420"            /* mental or court  */
	$"6F72 6465 7220 746F 2074 616B 6520 7375"            /* order to take su */
	$"6368 2061 6374 696F 6E3B 200A 2868 2920"            /* ch action; Â(h)  */
	$"414E 5343 4120 6861 7320 7265 6173 6F6E"            /* ANSCA has reason */
	$"2074 6F20 6265 6C69 6576 6520 7468 6174"            /*  to believe that */
	$"2073 7563 6820 6163 7469 6F6E 2069 7320"            /*  such action is  */
	$"7072 7564 656E 7420 6F72 206E 6563 6573"            /* prudent or neces */
	$"7361 7279 2E0A 0A35 2E20 436F 6E66 6964"            /* sary.ÂÂ5. Confid */
	$"656E 7469 616C 6974 792E 2022 436F 6E66"            /* entiality. "Conf */
	$"6964 656E 7469 616C 2049 6E66 6F72 6D61"            /* idential Informa */
	$"7469 6F6E 2220 7368 616C 6C20 6D65 616E"            /* tion" shall mean */
	$"2061 6C6C 2074 7261 6465 2073 6563 7265"            /*  all trade secre */
	$"7473 2C20 6B6E 6F77 2D68 6F77 2C20 696E"            /* ts, know-how, in */
	$"7665 6E74 696F 6E73 2C20 7465 6368 6E69"            /* ventions, techni */
	$"7175 6573 2C20 7072 6F63 6573 7365 732C"            /* ques, processes, */
	$"2061 6C67 6F72 6974 686D 732C 2073 6F66"            /*  algorithms, sof */
	$"7477 6172 6520 7072 6F67 7261 6D73 2C20"            /* tware programs,  */
	$"6861 7264 7761 7265 2C20 7363 6865 6D61"            /* hardware, schema */
	$"7469 6373 2C20 616E 6420 736F 6674 7761"            /* tics, and softwa */
	$"7265 2073 6F75 7263 6520 646F 6375 6D65"            /* re source docume */
	$"6E74 7320 7265 6C61 7469 6E67 2074 6F20"            /* nts relating to  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2C20 616E 6420 6F74 6865"            /* ftware, and othe */
	$"7220 696E 666F 726D 6174 696F 6E20 7072"            /* r information pr */
	$"6F76 6964 6564 2062 7920 414E 5343 412C"            /* ovided by ANSCA, */
	$"2077 6865 7468 6572 2064 6973 636C 6F73"            /*  whether disclos */
	$"6564 206F 7261 6C6C 792C 2069 6E20 7772"            /* ed orally, in wr */
	$"6974 696E 672C 206F 7220 6279 2065 7861"            /* iting, or by exa */
	$"6D69 6E61 7469 6F6E 206F 7220 696E 7370"            /* mination or insp */
	$"6563 7469 6F6E 2C20 6F74 6865 7220 7468"            /* ection, other th */
	$"616E 2069 6E66 6F72 6D61 7469 6F6E 2077"            /* an information w */
	$"6869 6368 2059 6F75 2063 616E 2064 656D"            /* hich You can dem */
	$"6F6E 7374 7261 7465 2028 6929 2077 6173"            /* onstrate (i) was */
	$"2061 6C72 6561 6479 206B 6E6F 776E 2074"            /*  already known t */
	$"6F20 596F 752C 206F 7468 6572 2074 6861"            /* o You, other tha */
	$"6E20 756E 6465 7220 616E 206F 626C 6967"            /* n under an oblig */
	$"6174 696F 6E20 6F66 2063 6F6E 6669 6465"            /* ation of confide */
	$"6E74 6961 6C69 7479 2C20 6174 2074 6865"            /* ntiality, at the */
	$"2074 696D 6520 6F66 2064 6973 636C 6F73"            /*  time of disclos */
	$"7572 653B 2028 6969 2920 7761 7320 6765"            /* ure; (ii) was ge */
	$"6E65 7261 6C6C 7920 6176 6169 6C61 626C"            /* nerally availabl */
	$"6520 696E 2074 6865 2070 7562 6C69 6320"            /* e in the public  */
	$"646F 6D61 696E 2061 7420 7468 6520 7469"            /* domain at the ti */
	$"6D65 206F 6620 6469 7363 6C6F 7375 7265"            /* me of disclosure */
	$"2074 6F20 596F 753B 2028 6969 6929 2062"            /*  to You; (iii) b */
	$"6563 616D 6520 6765 6E65 7261 6C6C 7920"            /* ecame generally  */
	$"6176 6169 6C61 626C 6520 696E 2074 6865"            /* available in the */
	$"2070 7562 6C69 6320 646F 6D61 696E 2061"            /*  public domain a */
	$"6674 6572 2064 6973 636C 6F73 7572 6520"            /* fter disclosure  */
	$"6F74 6865 7220 7468 616E 2074 6872 6F75"            /* other than throu */
	$"6768 2061 6E79 2061 6374 206F 7220 6F6D"            /* gh any act or om */
	$"6973 7369 6F6E 206F 6620 596F 753B 2028"            /* ission of You; ( */
	$"6976 2920 7761 7320 7375 6273 6571 7565"            /* iv) was subseque */
	$"6E74 6C79 206C 6177 6675 6C6C 7920 6469"            /* ntly lawfully di */
	$"7363 6C6F 7365 6420 746F 2059 6F75 2062"            /* sclosed to You b */
	$"7920 6120 7468 6972 6420 7061 7274 7920"            /* y a third party  */
	$"7769 7468 6F75 7420 616E 7920 6F62 6C69"            /* without any obli */
	$"6761 7469 6F6E 206F 6620 636F 6E66 6964"            /* gation of confid */
	$"656E 7469 616C 6974 793B 206F 7220 2876"            /* entiality; or (v */
	$"2920 7761 7320 696E 6465 7065 6E64 656E"            /* ) was independen */
	$"746C 7920 6465 7665 6C6F 7065 6420 6279"            /* tly developed by */
	$"2059 6F75 2077 6974 686F 7574 2075 7365"            /*  You without use */
	$"206F 6620 6F72 2072 6566 6572 656E 6365"            /*  of or reference */
	$"2074 6F20 616E 7920 696E 666F 726D 6174"            /*  to any informat */
	$"696F 6E20 6F72 206D 6174 6572 6961 6C73"            /* ion or materials */
	$"2064 6973 636C 6F73 6564 2062 7920 414E"            /*  disclosed by AN */
	$"5343 4120 6F72 2069 7473 2073 7570 706C"            /* SCA or its suppl */
	$"6965 7273 2E20 436F 6E66 6964 656E 7469"            /* iers. Confidenti */
	$"616C 2049 6E66 6F72 6D61 7469 6F6E 2073"            /* al Information s */
	$"6861 6C6C 2069 6E63 6C75 6465 2077 6974"            /* hall include wit */
	$"686F 7574 206C 696D 6974 6174 696F 6E20"            /* hout limitation  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2C20 5065 7266 6F72 6D61"            /* ftware, Performa */
	$"6E63 6520 4461 7461 2C20 616E 6420 616E"            /* nce Data, and an */
	$"7920 7570 6461 7465 732E 2059 6F75 2073"            /* y updates. You s */
	$"6861 6C6C 206E 6F74 2075 7365 2061 6E79"            /* hall not use any */
	$"2043 6F6E 6669 6465 6E74 6961 6C20 496E"            /*  Confidential In */
	$"666F 726D 6174 696F 6E20 666F 7220 616E"            /* formation for an */
	$"7920 7075 7270 6F73 6520 6F74 6865 7220"            /* y purpose other  */
	$"7468 616E 2061 7320 6578 7072 6573 736C"            /* than as expressl */
	$"7920 6175 7468 6F72 697A 6564 2075 6E64"            /* y authorized und */
	$"6572 2074 6869 7320 4C69 6365 6E73 652E"            /* er this License. */
	$"2049 6E20 6E6F 2065 7665 6E74 2073 6861"            /*  In no event sha */
	$"6C6C 2059 6F75 2075 7365 2074 6865 2044"            /* ll You use the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6F72 2061 6E79 2043 6F6E 6669 6465"            /* e or any Confide */
	$"6E74 6961 6C20 496E 666F 726D 6174 696F"            /* ntial Informatio */
	$"6E20 746F 2064 6576 656C 6F70 2C20 6D61"            /* n to develop, ma */
	$"6E75 6661 6374 7572 652C 206D 6172 6B65"            /* nufacture, marke */
	$"742C 2073 656C 6C2C 206F 7220 6469 7374"            /* t, sell, or dist */
	$"7269 6275 7465 2061 6E79 2070 726F 6475"            /* ribute any produ */
	$"6374 206F 7220 7365 7276 6963 652E 2059"            /* ct or service. Y */
	$"6F75 2073 6861 6C6C 206C 696D 6974 2064"            /* ou shall limit d */
	$"6973 7365 6D69 6E61 7469 6F6E 206F 6620"            /* issemination of  */
	$"436F 6E66 6964 656E 7469 616C 2049 6E66"            /* Confidential Inf */
	$"6F72 6D61 7469 6F6E 2074 6F20 6974 7320"            /* ormation to its  */
	$"656D 706C 6F79 6565 7320 7768 6F20 6861"            /* employees who ha */
	$"7665 2061 206E 6565 6420 746F 206B 6E6F"            /* ve a need to kno */
	$"7720 7375 6368 2043 6F6E 6669 6465 6E74"            /* w such Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E20"            /* ial Information  */
	$"666F 7220 7075 7270 6F73 6573 2065 7870"            /* for purposes exp */
	$"7265 7373 6C79 2061 7574 686F 7269 7A65"            /* ressly authorize */
	$"6420 756E 6465 7220 7468 6973 204C 6963"            /* d under this Lic */
	$"656E 7365 2E20 496E 206E 6F20 6576 656E"            /* ense. In no even */
	$"7420 7368 616C 6C20 596F 7520 6469 7363"            /* t shall You disc */
	$"6C6F 7365 2061 6E79 2043 6F6E 6669 6465"            /* lose any Confide */
	$"6E74 6961 6C20 496E 666F 726D 6174 696F"            /* ntial Informatio */
	$"6E20 746F 2061 6E79 2074 6869 7264 2070"            /* n to any third p */
	$"6172 7479 2E20 5769 7468 6F75 7420 6C69"            /* arty. Without li */
	$"6D69 7469 6E67 2074 6865 2066 6F72 6567"            /* miting the foreg */
	$"6F69 6E67 2C20 596F 7520 7368 616C 6C20"            /* oing, You shall  */
	$"7573 6520 6174 206C 6561 7374 2074 6865"            /* use at least the */
	$"2073 616D 6520 6465 6772 6565 206F 6620"            /*  same degree of  */
	$"6361 7265 2074 6861 7420 6974 2075 7365"            /* care that it use */
	$"7320 746F 2070 7265 7665 6E74 2074 6865"            /* s to prevent the */
	$"2064 6973 636C 6F73 7572 6520 6F66 2069"            /*  disclosure of i */
	$"7473 206F 776E 2063 6F6E 6669 6465 6E74"            /* ts own confident */
	$"6961 6C20 696E 666F 726D 6174 696F 6E20"            /* ial information  */
	$"6F66 206C 696B 6520 696D 706F 7274 616E"            /* of like importan */
	$"6365 2C20 6275 7420 696E 206E 6F20 6576"            /* ce, but in no ev */
	$"656E 7420 6C65 7373 2074 6861 6E20 7265"            /* ent less than re */
	$"6173 6F6E 6162 6C65 2063 6172 652C 2074"            /* asonable care, t */
	$"6F20 7072 6576 656E 7420 7468 6520 6469"            /* o prevent the di */
	$"7363 6C6F 7375 7265 206F 6620 436F 6E66"            /* sclosure of Conf */
	$"6964 656E 7469 616C 2049 6E66 6F72 6D61"            /* idential Informa */
	$"7469 6F6E 2E0A 0A36 2E20 5761 7272 616E"            /* tion.ÂÂ6. Warran */
	$"7479 2044 6973 636C 6169 6D65 722E 2049"            /* ty Disclaimer. I */
	$"5420 4953 2055 4E44 4552 5354 4F4F 4420"            /* T IS UNDERSTOOD  */
	$"5448 4154 2054 4845 2044 4556 454C 4F50"            /* THAT THE DEVELOP */
	$"4552 2053 4F46 5457 4152 4520 414E 4420"            /* ER SOFTWARE AND  */
	$"414E 5920 5550 4441 5445 5320 4D41 5920"            /* ANY UPDATES MAY  */
	$"434F 4E54 4149 4E20 4552 524F 5253 2041"            /* CONTAIN ERRORS A */
	$"4E44 2041 5245 2050 524F 5649 4445 4420"            /* ND ARE PROVIDED  */
	$"464F 5220 4C49 4D49 5445 4420 4556 414C"            /* FOR LIMITED EVAL */
	$"5541 5449 4F4E 204F 4E4C 592E 2054 4845"            /* UATION ONLY. THE */
	$"2044 4556 454C 4F50 4552 2053 4F46 5457"            /*  DEVELOPER SOFTW */
	$"4152 4520 414E 4420 414E 5920 5550 4441"            /* ARE AND ANY UPDA */
	$"5445 5320 4152 4520 5052 4F56 4944 4544"            /* TES ARE PROVIDED */
	$"2022 4153 2049 5322 2057 4954 484F 5554"            /*  "AS IS" WITHOUT */
	$"2057 4152 5241 4E54 5920 4F46 2041 4E59"            /*  WARRANTY OF ANY */
	$"204B 494E 442C 2057 4845 5448 4552 2045"            /*  KIND, WHETHER E */
	$"5850 5245 5353 2C20 494D 504C 4945 442C"            /* XPRESS, IMPLIED, */
	$"2053 5441 5455 544F 5259 2C20 4F52 204F"            /*  STATUTORY, OR O */
	$"5448 4552 5749 5345 2E20 414E 5343 4120"            /* THERWISE. ANSCA  */
	$"414E 4420 4954 5320 5355 5050 4C49 4552"            /* AND ITS SUPPLIER */
	$"5320 5350 4543 4946 4943 414C 4C59 2044"            /* S SPECIFICALLY D */
	$"4953 434C 4149 4D20 414C 4C20 494D 504C"            /* ISCLAIM ALL IMPL */
	$"4945 4420 5741 5252 414E 5449 4553 204F"            /* IED WARRANTIES O */
	$"4620 4D45 5243 4841 4E54 4142 494C 4954"            /* F MERCHANTABILIT */
	$"592C 204E 4F4E 494E 4652 494E 4745 4D45"            /* Y, NONINFRINGEME */
	$"4E54 2C20 414E 4420 4649 544E 4553 5320"            /* NT, AND FITNESS  */
	$"464F 5220 4120 5041 5254 4943 554C 4152"            /* FOR A PARTICULAR */
	$"2050 5552 504F 5345 2E20 596F 7520 6163"            /*  PURPOSE. You ac */
	$"6B6E 6F77 6C65 6467 6520 7468 6174 2041"            /* knowledge that A */
	$"4E53 4341 2068 6173 206E 6F74 2070 7562"            /* NSCA has not pub */
	$"6C69 636C 7920 616E 6E6F 756E 6365 6420"            /* licly announced  */
	$"7468 6520 6176 6169 6C61 6269 6C69 7479"            /* the availability */
	$"206F 6620 7468 6520 4465 7665 6C6F 7065"            /*  of the Develope */
	$"7220 536F 6674 7761 7265 2C20 7468 6174"            /* r Software, that */
	$"2041 4E53 4341 2068 6173 206E 6F74 2070"            /*  ANSCA has not p */
	$"726F 6D69 7365 6420 6F72 2067 7561 7261"            /* romised or guara */
	$"6E74 6565 6420 746F 2059 6F75 2074 6861"            /* nteed to You tha */
	$"7420 7375 6368 2044 6576 656C 6F70 6572"            /* t such Developer */
	$"2053 6F66 7477 6172 6520 7769 6C6C 2062"            /*  Software will b */
	$"6520 616E 6E6F 756E 6365 6420 6F72 206D"            /* e announced or m */
	$"6164 6520 6176 6169 6C61 626C 6520 746F"            /* ade available to */
	$"2061 6E79 6F6E 6520 696E 2074 6865 2066"            /*  anyone in the f */
	$"7574 7572 652C 2074 6861 7420 414E 5343"            /* uture, that ANSC */
	$"4120 6861 7320 6E6F 2065 7870 7265 7373"            /* A has no express */
	$"206F 7220 696D 706C 6965 6420 6F62 6C69"            /*  or implied obli */
	$"6761 7469 6F6E 2074 6F20 596F 7520 746F"            /* gation to You to */
	$"2061 6E6E 6F75 6E63 6520 6F72 2069 6E74"            /*  announce or int */
	$"726F 6475 6365 2074 6865 2044 6576 656C"            /* roduce the Devel */
	$"6F70 6572 2053 6F66 7477 6172 652C 2061"            /* oper Software, a */
	$"6E64 2074 6861 7420 414E 5343 4120 6D61"            /* nd that ANSCA ma */
	$"7920 6E6F 7420 696E 7472 6F64 7563 6520"            /* y not introduce  */
	$"6120 7072 6F64 7563 7420 7369 6D69 6C61"            /* a product simila */
	$"7220 6F72 2063 6F6D 7061 7469 626C 6520"            /* r or compatible  */
	$"7769 7468 2074 6865 2044 6576 656C 6F70"            /* with the Develop */
	$"6572 2053 6F66 7477 6172 652E 2041 6363"            /* er Software. Acc */
	$"6F72 6469 6E67 6C79 2C20 596F 7520 6163"            /* ordingly, You ac */
	$"6B6E 6F77 6C65 6467 6520 7468 6174 2061"            /* knowledge that a */
	$"6E79 2072 6573 6561 7263 6820 6F72 2064"            /* ny research or d */
	$"6576 656C 6F70 6D65 6E74 2074 6861 7420"            /* evelopment that  */
	$"6974 2070 6572 666F 726D 7320 7265 6761"            /* it performs rega */
	$"7264 696E 6720 7468 6520 4465 7665 6C6F"            /* rding the Develo */
	$"7065 7220 536F 6674 7761 7265 206F 7220"            /* per Software or  */
	$"616E 7920 7072 6F64 7563 7420 6173 736F"            /* any product asso */
	$"6369 6174 6564 2077 6974 6820 7468 6520"            /* ciated with the  */
	$"4465 7665 6C6F 7065 7220 536F 6674 7761"            /* Developer Softwa */
	$"7265 2069 7320 646F 6E65 2065 6E74 6972"            /* re is done entir */
	$"656C 7920 6174 2059 6F75 7220 6F77 6E20"            /* ely at Your own  */
	$"7269 736B 2E20 5370 6563 6966 6963 616C"            /* risk. Specifical */
	$"6C79 2C20 7468 6520 4465 7665 6C6F 7065"            /* ly, the Develope */
	$"7220 536F 6674 7761 7265 206D 6179 2063"            /* r Software may c */
	$"6F6E 7461 696E 2066 6561 7475 7265 732C"            /* ontain features, */
	$"2066 756E 6374 696F 6E61 6C69 7479 206F"            /*  functionality o */
	$"7220 6D6F 6475 6C65 7320 7468 6174 2077"            /* r modules that w */
	$"696C 6C20 6E6F 7420 6265 2069 6E63 6C75"            /* ill not be inclu */
	$"6465 6420 696E 2074 6865 2070 726F 6475"            /* ded in the produ */
	$"6374 696F 6E20 7665 7273 696F 6E20 6F66"            /* ction version of */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 652C 2069 6620 7265 6C65"            /* oftware, if rele */
	$"6173 6564 2C20 6F72 2074 6861 7420 7769"            /* ased, or that wi */
	$"6C6C 2062 6520 6D61 726B 6574 6564 2073"            /* ll be marketed s */
	$"6570 6172 6174 656C 7920 666F 7220 6164"            /* eparately for ad */
	$"6469 7469 6F6E 616C 2066 6565 732E 0A0A"            /* ditional fees.ÂÂ */
	$"372E 204C 696D 6974 6174 696F 6E20 6F66"            /* 7. Limitation of */
	$"204C 6961 6269 6C69 7479 2E20 544F 2054"            /*  Liability. TO T */
	$"4845 2045 5854 454E 5420 4E4F 5420 5052"            /* HE EXTENT NOT PR */
	$"4F48 4942 4954 4544 2042 5920 4C41 572C"            /* OHIBITED BY LAW, */
	$"2049 4E20 4E4F 2045 5645 4E54 2053 4841"            /*  IN NO EVENT SHA */
	$"4C4C 2041 4E53 4341 2042 4520 4C49 4142"            /* LL ANSCA BE LIAB */
	$"4C45 2046 4F52 2050 4552 534F 4E41 4C20"            /* LE FOR PERSONAL  */
	$"494E 4A55 5259 2C20 4F52 2041 4E59 2049"            /* INJURY, OR ANY I */
	$"4E43 4944 454E 5441 4C2C 2053 5045 4349"            /* NCIDENTAL, SPECI */
	$"414C 2C20 494E 4449 5245 4354 204F 5220"            /* AL, INDIRECT OR  */
	$"434F 4E53 4551 5545 4E54 4941 4C20 4441"            /* CONSEQUENTIAL DA */
	$"4D41 4745 5320 5748 4154 534F 4556 4552"            /* MAGES WHATSOEVER */
	$"2C20 494E 434C 5544 494E 472C 2057 4954"            /* , INCLUDING, WIT */
	$"484F 5554 204C 494D 4954 4154 494F 4E2C"            /* HOUT LIMITATION, */
	$"2044 414D 4147 4553 2046 4F52 204C 4F53"            /*  DAMAGES FOR LOS */
	$"5320 4F46 2050 524F 4649 5453 2C20 434F"            /* S OF PROFITS, CO */
	$"5252 5550 5449 4F4E 204F 5220 4C4F 5353"            /* RRUPTION OR LOSS */
	$"204F 4620 4441 5441 2C20 4255 5349 4E45"            /*  OF DATA, BUSINE */
	$"5353 2049 4E54 4552 5255 5054 494F 4E20"            /* SS INTERRUPTION  */
	$"4F52 2041 4E59 204F 5448 4552 2043 4F4D"            /* OR ANY OTHER COM */
	$"4D45 5243 4941 4C20 4441 4D41 4745 5320"            /* MERCIAL DAMAGES  */
	$"4F52 204C 4F53 5345 532C 2041 5249 5349"            /* OR LOSSES, ARISI */
	$"4E47 204F 5554 204F 4620 4F52 2052 454C"            /* NG OUT OF OR REL */
	$"4154 4544 2054 4F20 594F 5552 2055 5345"            /* ATED TO YOUR USE */
	$"204F 5220 494E 4142 494C 4954 5920 544F"            /*  OR INABILITY TO */
	$"2055 5345 2054 4845 2044 4556 454C 4F50"            /*  USE THE DEVELOP */
	$"4552 2053 4F46 5457 4152 452C 2048 4F57"            /* ER SOFTWARE, HOW */
	$"4556 4552 2043 4155 5345 442C 2052 4547"            /* EVER CAUSED, REG */
	$"4152 444C 4553 5320 4F46 2054 4845 2054"            /* ARDLESS OF THE T */
	$"4845 4F52 5920 4F46 204C 4941 4249 4C49"            /* HEORY OF LIABILI */
	$"5459 2028 434F 4E54 5241 4354 2C20 544F"            /* TY (CONTRACT, TO */
	$"5254 204F 5220 4F54 4845 5257 4953 4529"            /* RT OR OTHERWISE) */
	$"2041 4E44 2045 5645 4E20 4946 2041 4E53"            /*  AND EVEN IF ANS */
	$"4341 2048 4153 2042 4545 4E20 4144 5649"            /* CA HAS BEEN ADVI */
	$"5345 4420 4F46 2054 4845 2050 4F53 5349"            /* SED OF THE POSSI */
	$"4249 4C49 5459 204F 4620 5355 4348 2044"            /* BILITY OF SUCH D */
	$"414D 4147 4553 2E20 534F 4D45 204A 5552"            /* AMAGES. SOME JUR */
	$"4953 4449 4354 494F 4E53 2044 4F20 4E4F"            /* ISDICTIONS DO NO */
	$"5420 414C 4C4F 5720 5448 4520 4C49 4D49"            /* T ALLOW THE LIMI */
	$"5441 5449 4F4E 204F 4620 4C49 4142 494C"            /* TATION OF LIABIL */
	$"4954 5920 464F 5220 5045 5253 4F4E 414C"            /* ITY FOR PERSONAL */
	$"2049 4E4A 5552 592C 204F 5220 4F46 2049"            /*  INJURY, OR OF I */
	$"4E43 4944 454E 5441 4C20 4F52 2043 4F4E"            /* NCIDENTAL OR CON */
	$"5345 5155 454E 5449 414C 2044 414D 4147"            /* SEQUENTIAL DAMAG */
	$"4553 2C20 534F 2054 4849 5320 4C49 4D49"            /* ES, SO THIS LIMI */
	$"5441 5449 4F4E 204D 4159 204E 4F54 2041"            /* TATION MAY NOT A */
	$"5050 4C59 2054 4F20 594F 552E 2049 6E20"            /* PPLY TO YOU. In  */
	$"6E6F 2065 7665 6E74 2073 6861 6C6C 2041"            /* no event shall A */
	$"4E53 4341 2773 2074 6F74 616C 206C 6961"            /* NSCA's total lia */
	$"6269 6C69 7479 2074 6F20 596F 7520 666F"            /* bility to You fo */
	$"7220 616C 6C20 6461 6D61 6765 7320 286F"            /* r all damages (o */
	$"7468 6572 2074 6861 6E20 6173 206D 6179"            /* ther than as may */
	$"2062 6520 7265 7175 6972 6564 2062 7920"            /*  be required by  */
	$"6170 706C 6963 6162 6C65 206C 6177 2069"            /* applicable law i */
	$"6E20 6361 7365 7320 696E 766F 6C76 696E"            /* n cases involvin */
	$"6720 7065 7273 6F6E 616C 2069 6E6A 7572"            /* g personal injur */
	$"7929 2065 7863 6565 6420 7468 6520 616D"            /* y) exceed the am */
	$"6F75 6E74 206F 6620 6669 6674 7920 646F"            /* ount of fifty do */
	$"6C6C 6172 7320 2824 3530 2E30 3029 2E20"            /* llars ($50.00).  */
	$"2054 6865 2066 6F72 6567 6F69 6E67 206C"            /*  The foregoing l */
	$"696D 6974 6174 696F 6E73 2077 696C 6C20"            /* imitations will  */
	$"6170 706C 7920 6576 656E 2069 6620 7468"            /* apply even if th */
	$"6520 6162 6F76 6520 7374 6174 6564 2072"            /* e above stated r */
	$"656D 6564 7920 6661 696C 7320 6F66 2069"            /* emedy fails of i */
	$"7473 2065 7373 656E 7469 616C 2070 7572"            /* ts essential pur */
	$"706F 7365 2E0A 0A38 2E20 4578 706F 7274"            /* pose.ÂÂ8. Export */
	$"2043 6F6E 7472 6F6C 2E20 596F 7520 6D61"            /*  Control. You ma */
	$"7920 6E6F 7420 7573 6520 6F72 206F 7468"            /* y not use or oth */
	$"6572 7769 7365 2065 7870 6F72 7420 6F72"            /* erwise export or */
	$"2072 6565 7870 6F72 7420 7468 6520 4465"            /*  reexport the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2065 7863 6570 7420 6173 2061 7574 686F"            /*  except as autho */
	$"7269 7A65 6420 6279 2055 6E69 7465 6420"            /* rized by United  */
	$"5374 6174 6573 206C 6177 2061 6E64 2074"            /* States law and t */
	$"6865 206C 6177 7320 6F66 2074 6865 206A"            /* he laws of the j */
	$"7572 6973 6469 6374 696F 6E28 7329 2069"            /* urisdiction(s) i */
	$"6E20 7768 6963 6820 7468 6520 4465 7665"            /* n which the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2077"            /* loper Software w */
	$"6173 206F 6274 6169 6E65 642E 2049 6E20"            /* as obtained. In  */
	$"7061 7274 6963 756C 6172 2C20 6275 7420"            /* particular, but  */
	$"7769 7468 6F75 7420 6C69 6D69 7461 7469"            /* without limitati */
	$"6F6E 2C20 7468 6520 4465 7665 6C6F 7065"            /* on, the Develope */
	$"7220 536F 6674 7761 7265 206D 6179 206E"            /* r Software may n */
	$"6F74 2062 6520 6578 706F 7274 6564 206F"            /* ot be exported o */
	$"7220 7265 2D65 7870 6F72 7465 6420 2861"            /* r re-exported (a */
	$"2920 696E 746F 2061 6E79 2055 2E53 2E20"            /* ) into any U.S.  */
	$"656D 6261 7267 6F65 6420 636F 756E 7472"            /* embargoed countr */
	$"6965 7320 6F72 2028 6229 2074 6F20 616E"            /* ies or (b) to an */
	$"796F 6E65 206F 6E20 7468 6520 552E 532E"            /* yone on the U.S. */
	$"2054 7265 6173 7572 7920 4465 7061 7274"            /*  Treasury Depart */
	$"6D65 6E74 2773 206C 6973 7420 6F66 2053"            /* ment's list of S */
	$"7065 6369 616C 6C79 2044 6573 6967 6E61"            /* pecially Designa */
	$"7465 6420 4E61 7469 6F6E 616C 7320 6F72"            /* ted Nationals or */
	$"2074 6865 2055 2E53 2E20 4465 7061 7274"            /*  the U.S. Depart */
	$"6D65 6E74 206F 6620 436F 6D6D 6572 6365"            /* ment of Commerce */
	$"2044 656E 6965 6420 5065 7273 6F6E 2773"            /*  Denied Person's */
	$"204C 6973 7420 6F72 2045 6E74 6974 7920"            /*  List or Entity  */
	$"4C69 7374 2E20 4279 2075 7369 6E67 2074"            /* List. By using t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 652C 2059 6F75 2072 6570 7265"            /* tware, You repre */
	$"7365 6E74 2061 6E64 2077 6172 7261 6E74"            /* sent and warrant */
	$"2074 6861 7420 596F 7520 6172 6520 6E6F"            /*  that You are no */
	$"7420 6C6F 6361 7465 6420 696E 2061 6E79"            /* t located in any */
	$"2073 7563 6820 636F 756E 7472 7920 6F72"            /*  such country or */
	$"206F 6E20 616E 7920 7375 6368 206C 6973"            /*  on any such lis */
	$"742E 2059 6F75 2061 6C73 6F20 6167 7265"            /* t. You also agre */
	$"6520 7468 6174 2059 6F75 2077 696C 6C20"            /* e that You will  */
	$"6E6F 7420 7573 6520 7468 6520 4465 7665"            /* not use the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2066"            /* loper Software f */
	$"6F72 2061 6E79 2070 7572 706F 7365 7320"            /* or any purposes  */
	$"7072 6F68 6962 6974 6564 2062 7920 556E"            /* prohibited by Un */
	$"6974 6564 2053 7461 7465 7320 6C61 772C"            /* ited States law, */
	$"2069 6E63 6C75 6469 6E67 2C20 7769 7468"            /*  including, with */
	$"6F75 7420 6C69 6D69 7461 7469 6F6E 2C20"            /* out limitation,  */
	$"7468 6520 6465 7665 6C6F 706D 656E 742C"            /* the development, */
	$"2064 6573 6967 6E2C 206D 616E 7566 6163"            /*  design, manufac */
	$"7475 7265 206F 7220 7072 6F64 7563 7469"            /* ture or producti */
	$"6F6E 206F 6620 6E75 636C 6561 722C 2063"            /* on of nuclear, c */
	$"6865 6D69 6361 6C20 6F72 2062 696F 6C6F"            /* hemical or biolo */
	$"6769 6361 6C20 7765 6170 6F6E 732E 0A0A"            /* gical weapons.ÂÂ */
	$"392E 2047 6F76 6572 6E6D 656E 7420 456E"            /* 9. Government En */
	$"6420 5573 6572 732E 2054 6865 2044 6576"            /* d Users. The Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"616E 6420 7265 6C61 7465 6420 646F 6375"            /* and related docu */
	$"6D65 6E74 6174 696F 6E20 6172 6520 2243"            /* mentation are "C */
	$"6F6D 6D65 7263 6961 6C20 4974 656D 7322"            /* ommercial Items" */
	$"2C20 6173 2074 6861 7420 7465 726D 2069"            /* , as that term i */
	$"7320 6465 6669 6E65 6420 6174 2034 3820"            /* s defined at 48  */
	$"432E 462E 522E 20C2 A732 2E31 3031 2C20"            /* C.F.R. Â§2.101,  */
	$"636F 6E73 6973 7469 6E67 206F 6620 2243"            /* consisting of "C */
	$"6F6D 6D65 7263 6961 6C20 436F 6D70 7574"            /* ommercial Comput */
	$"6572 2053 6F66 7477 6172 6522 2061 6E64"            /* er Software" and */
	$"2022 436F 6D6D 6572 6369 616C 2043 6F6D"            /*  "Commercial Com */
	$"7075 7465 7220 536F 6674 7761 7265 2044"            /* puter Software D */
	$"6F63 756D 656E 7461 7469 6F6E 222C 2061"            /* ocumentation", a */
	$"7320 7375 6368 2074 6572 6D73 2061 7265"            /* s such terms are */
	$"2075 7365 6420 696E 2034 3820 432E 462E"            /*  used in 48 C.F. */
	$"522E 20C2 A731 322E 3231 3220 6F72 2034"            /* R. Â§12.212 or 4 */
	$"3820 432E 462E 522E 20C2 A732 3237 2E37"            /* 8 C.F.R. Â§227.7 */
	$"3230 322C 2061 7320 6170 706C 6963 6162"            /* 202, as applicab */
	$"6C65 2E20 436F 6E73 6973 7465 6E74 2077"            /* le. Consistent w */
	$"6974 6820 3438 2043 2E46 2E52 2E20 C2A7"            /* ith 48 C.F.R. Â§ */
	$"3132 2E32 3132 206F 7220 3438 2043 2E46"            /* 12.212 or 48 C.F */
	$"2E52 2E20 C2A7 3232 372E 3732 3032 2D31"            /* .R. Â§227.7202-1 */
	$"2074 6872 6F75 6768 2032 3237 2E37 3230"            /*  through 227.720 */
	$"322D 342C 2061 7320 6170 706C 6963 6162"            /* 2-4, as applicab */
	$"6C65 2C20 7468 6520 436F 6D6D 6572 6369"            /* le, the Commerci */
	$"616C 2043 6F6D 7075 7465 7220 536F 6674"            /* al Computer Soft */
	$"7761 7265 2061 6E64 2043 6F6D 6D65 7263"            /* ware and Commerc */
	$"6961 6C20 436F 6D70 7574 6572 2053 6F66"            /* ial Computer Sof */
	$"7477 6172 6520 446F 6375 6D65 6E74 6174"            /* tware Documentat */
	$"696F 6E20 6172 6520 6265 696E 6720 6C69"            /* ion are being li */
	$"6365 6E73 6564 2074 6F20 552E 532E 2047"            /* censed to U.S. G */
	$"6F76 6572 6E6D 656E 7420 656E 6420 7573"            /* overnment end us */
	$"6572 7320 2861 2920 6F6E 6C79 2061 7320"            /* ers (a) only as  */
	$"436F 6D6D 6572 6369 616C 2049 7465 6D73"            /* Commercial Items */
	$"2061 6E64 2028 6229 2077 6974 6820 6F6E"            /*  and (b) with on */
	$"6C79 2074 686F 7365 2072 6967 6874 7320"            /* ly those rights  */
	$"6173 2061 7265 2067 7261 6E74 6564 2074"            /* as are granted t */
	$"6F20 616C 6C20 6F74 6865 7220 656E 6420"            /* o all other end  */
	$"7573 6572 7320 7075 7273 7561 6E74 2074"            /* users pursuant t */
	$"6F20 7468 6520 7465 726D 7320 616E 6420"            /* o the terms and  */
	$"636F 6E64 6974 696F 6E73 2068 6572 6569"            /* conditions herei */
	$"6E2E 2055 6E70 7562 6C69 7368 6564 2D72"            /* n. Unpublished-r */
	$"6967 6874 7320 7265 7365 7276 6564 2075"            /* ights reserved u */
	$"6E64 6572 2074 6865 2063 6F70 7972 6967"            /* nder the copyrig */
	$"6874 206C 6177 7320 6F66 2074 6865 2055"            /* ht laws of the U */
	$"6E69 7465 6420 5374 6174 6573 2E0A 0A31"            /* nited States.ÂÂ1 */
	$"302E 2043 6F6E 7472 6F6C 6C69 6E67 204C"            /* 0. Controlling L */
	$"6177 2061 6E64 2053 6576 6572 6162 696C"            /* aw and Severabil */
	$"6974 792E 2054 6869 7320 4C69 6365 6E73"            /* ity. This Licens */
	$"6520 7769 6C6C 2062 6520 676F 7665 726E"            /* e will be govern */
	$"6564 2062 7920 616E 6420 636F 6E73 7472"            /* ed by and constr */
	$"7565 6420 696E 2061 6363 6F72 6461 6E63"            /* ued in accordanc */
	$"6520 7769 7468 2074 6865 206C 6177 7320"            /* e with the laws  */
	$"6F66 2074 6865 2053 7461 7465 206F 6620"            /* of the State of  */
	$"4361 6C69 666F 726E 6961 2C20 6173 2061"            /* California, as a */
	$"7070 6C69 6564 2074 6F20 6167 7265 656D"            /* pplied to agreem */
	$"656E 7473 2065 6E74 6572 6564 2069 6E74"            /* ents entered int */
	$"6F20 616E 6420 746F 2062 6520 7065 7266"            /* o and to be perf */
	$"6F72 6D65 6420 656E 7469 7265 6C79 2077"            /* ormed entirely w */
	$"6974 6869 6E20 4361 6C69 666F 726E 6961"            /* ithin California */
	$"2062 6574 7765 656E 2043 616C 6966 6F72"            /*  between Califor */
	$"6E69 6120 7265 7369 6465 6E74 732E 2054"            /* nia residents. T */
	$"6869 7320 4C69 6365 6E73 6520 7368 616C"            /* his License shal */
	$"6C20 6E6F 7420 6265 2067 6F76 6572 6E65"            /* l not be governe */
	$"6420 6279 2074 6865 2055 6E69 7465 6420"            /* d by the United  */
	$"4E61 7469 6F6E 7320 436F 6E76 656E 7469"            /* Nations Conventi */
	$"6F6E 206F 6E20 436F 6E74 7261 6374 7320"            /* on on Contracts  */
	$"666F 7220 7468 6520 496E 7465 726E 6174"            /* for the Internat */
	$"696F 6E61 6C20 5361 6C65 206F 6620 476F"            /* ional Sale of Go */
	$"6F64 732C 2074 6865 2061 7070 6C69 6361"            /* ods, the applica */
	$"7469 6F6E 206F 6620 7768 6963 6820 6973"            /* tion of which is */
	$"2065 7870 7265 7373 6C79 2065 7863 6C75"            /*  expressly exclu */
	$"6465 642E 2049 6620 666F 7220 616E 7920"            /* ded. If for any  */
	$"7265 6173 6F6E 2061 2063 6F75 7274 206F"            /* reason a court o */
	$"6620 636F 6D70 6574 656E 7420 6A75 7269"            /* f competent juri */
	$"7364 6963 7469 6F6E 2066 696E 6473 2061"            /* sdiction finds a */
	$"6E79 2070 726F 7669 7369 6F6E 2C20 6F72"            /* ny provision, or */
	$"2070 6F72 7469 6F6E 2074 6865 7265 6F66"            /*  portion thereof */
	$"2C20 746F 2062 6520 756E 656E 666F 7263"            /* , to be unenforc */
	$"6561 626C 652C 2074 6865 2072 656D 6169"            /* eable, the remai */
	$"6E64 6572 206F 6620 7468 6973 204C 6963"            /* nder of this Lic */
	$"656E 7365 2073 6861 6C6C 2063 6F6E 7469"            /* ense shall conti */
	$"6E75 6520 696E 2066 756C 6C20 666F 7263"            /* nue in full forc */
	$"6520 616E 6420 6566 6665 6374 2E20 200A"            /* e and effect.  Â */
	$"0A31 312E 2043 6F6D 706C 6574 6520 4167"            /* Â11. Complete Ag */
	$"7265 656D 656E 743B 2047 6F76 6572 6E69"            /* reement; Governi */
	$"6E67 204C 616E 6775 6167 652E 2054 6869"            /* ng Language. Thi */
	$"7320 4C69 6365 6E73 6520 636F 6E73 7469"            /* s License consti */
	$"7475 7465 7320 7468 6520 656E 7469 7265"            /* tutes the entire */
	$"2061 6772 6565 6D65 6E74 2062 6574 7765"            /*  agreement betwe */
	$"656E 2074 6865 2070 6172 7469 6573 2077"            /* en the parties w */
	$"6974 6820 7265 7370 6563 7420 746F 2074"            /* ith respect to t */
	$"6865 2075 7365 206F 6620 7468 6520 4465"            /* he use of the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"206C 6963 656E 7365 6420 6865 7265 756E"            /*  licensed hereun */
	$"6465 7220 616E 6420 7375 7065 7273 6564"            /* der and supersed */
	$"6573 2061 6C6C 2070 7269 6F72 206F 7220"            /* es all prior or  */
	$"636F 6E74 656D 706F 7261 6E65 6F75 7320"            /* contemporaneous  */
	$"756E 6465 7273 7461 6E64 696E 6773 2072"            /* understandings r */
	$"6567 6172 6469 6E67 2073 7563 6820 7375"            /* egarding such su */
	$"626A 6563 7420 6D61 7474 6572 2E20 4E6F"            /* bject matter. No */
	$"2061 6D65 6E64 6D65 6E74 2074 6F20 6F72"            /*  amendment to or */
	$"206D 6F64 6966 6963 6174 696F 6E20 6F66"            /*  modification of */
	$"2074 6869 7320 4C69 6365 6E73 6520 7769"            /*  this License wi */
	$"6C6C 2062 6520 6269 6E64 696E 6720 756E"            /* ll be binding un */
	$"6C65 7373 2069 6E20 7772 6974 696E 6720"            /* less in writing  */
	$"616E 6420 7369 676E 6564 2062 7920 414E"            /* and signed by AN */
	$"5343 412E 2041 6E79 2074 7261 6E73 6C61"            /* SCA. Any transla */
	$"7469 6F6E 206F 6620 7468 6973 2041 6772"            /* tion of this Agr */
	$"6565 6D65 6E74 2069 7320 646F 6E65 2066"            /* eement is done f */
	$"6F72 206C 6F63 616C 2072 6571 7569 7265"            /* or local require */
	$"6D65 6E74 7320 616E 6420 696E 2074 6865"            /* ments and in the */
	$"2065 7665 6E74 206F 6620 6120 6469 7370"            /*  event of a disp */
	$"7574 6520 6265 7477 6565 6E20 7468 6520"            /* ute between the  */
	$"456E 676C 6973 6820 616E 6420 616E 7920"            /* English and any  */
	$"6E6F 6E2D 456E 676C 6973 6820 7665 7273"            /* non-English vers */
	$"696F 6E2C 2074 6865 2045 6E67 6C69 7368"            /* ion, the English */
	$"2076 6572 7369 6F6E 206F 6620 7468 6973"            /*  version of this */
	$"2041 6772 6565 6D65 6E74 2073 6861 6C6C"            /*  Agreement shall */
	$"2067 6F76 6572 6E2E 0A0A 3132 2E20 5468"            /*  govern.ÂÂ12. Th */
	$"6972 6420 5061 7274 7920 4163 6B6E 6F77"            /* ird Party Acknow */
	$"6C65 6467 656D 656E 7473 2E20 506F 7274"            /* ledgements. Port */
	$"696F 6E73 206F 6620 7468 6520 4465 7665"            /* ions of the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2075"            /* loper Software u */
	$"7469 6C69 7A65 206F 7220 696E 636C 7564"            /* tilize or includ */
	$"6520 6365 7274 6169 6E20 736F 6674 7761"            /* e certain softwa */
	$"7265 206C 6962 7261 7269 6573 2C20 7468"            /* re libraries, th */
	$"6972 6420 7061 7274 7920 736F 6674 7761"            /* ird party softwa */
	$"7265 2C20 616E 6420 6F74 6865 7220 636F"            /* re, and other co */
	$"7079 7269 6768 7465 6420 6D61 7465 7269"            /* pyrighted materi */
	$"616C 2E20 4163 6B6E 6F77 6C65 6467 656D"            /* al. Acknowledgem */
	$"656E 7473 2C20 6C69 6365 6E73 696E 6720"            /* ents, licensing  */
	$"7465 726D 7320 616E 6420 6469 7363 6C61"            /* terms and discla */
	$"696D 6572 7320 666F 7220 7375 6368 206D"            /* imers for such m */
	$"6174 6572 6961 6C20 6172 6520 636F 6E74"            /* aterial are cont */
	$"6169 6E65 6420 696E 2074 6865 2065 6C65"            /* ained in the ele */
	$"6374 726F 6E69 6320 646F 6375 6D65 6E74"            /* ctronic document */
	$"6174 696F 6E20 666F 7220 7468 6520 4465"            /* ation for the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"206F 7220 6D61 7920 6F74 6865 7277 6973"            /*  or may otherwis */
	$"6520 6163 636F 6D70 616E 7920 7375 6368"            /* e accompany such */
	$"206D 6174 6572 6961 6C2C 2061 6E64 2059"            /*  material, and Y */
	$"6F75 7220 7573 6520 6F66 2073 7563 6820"            /* our use of such  */
	$"6D61 7465 7269 616C 2069 7320 676F 7665"            /* material is gove */
	$"726E 6564 2062 7920 7468 6569 7220 7265"            /* rned by their re */
	$"7370 6563 7469 7665 2074 6572 6D73 2E0A"            /* spective terms.Â */
	$"0A0A 3230 3039 2D30 342D 3037"                      /* ÂÂ2009-04-07 */
};

