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
	$"414E 5343 4120 494E 432E 0A53 4F46 5457"            /* ANSCA INC.ÂSOFTW */
	$"4152 4520 4C49 4345 4E53 4520 4147 5245"            /* ARE LICENSE AGRE */
	$"454D 454E 540A 0A50 4C45 4153 4520 5245"            /* EMENTÂÂPLEASE RE */
	$"4144 2054 4849 5320 534F 4654 5741 5245"            /* AD THIS SOFTWARE */
	$"204C 4943 454E 5345 2041 4752 4545 4D45"            /*  LICENSE AGREEME */
	$"4E54 2028 224C 4943 454E 5345 2220 4153"            /* NT ("LICENSE" AS */
	$"2044 4546 494E 4544 2042 454C 4F57 2920"            /*  DEFINED BELOW)  */
	$"4341 5245 4655 4C4C 5920 4245 464F 5245"            /* CAREFULLY BEFORE */
	$"2055 5349 4E47 2054 4845 2044 4556 454C"            /*  USING THE DEVEL */
	$"4F50 4552 2053 4F46 5457 4152 4520 2844"            /* OPER SOFTWARE (D */
	$"4546 494E 4544 2042 454C 4F57 292E 2042"            /* EFINED BELOW). B */
	$"5920 5553 494E 4720 5448 4520 4445 5645"            /* Y USING THE DEVE */
	$"4C4F 5045 5220 534F 4654 5741 5245 2C20"            /* LOPER SOFTWARE,  */
	$"594F 5520 4152 4520 4147 5245 4549 4E47"            /* YOU ARE AGREEING */
	$"2054 4F20 4245 2042 4F55 4E44 2042 5920"            /*  TO BE BOUND BY  */
	$"5448 4520 5445 524D 5320 4F46 2054 4849"            /* THE TERMS OF THI */
	$"5320 4C49 4345 4E53 452E 2049 4620 594F"            /* S LICENSE. IF YO */
	$"5520 444F 204E 4F54 2041 4752 4545 2054"            /* U DO NOT AGREE T */
	$"4F20 5448 4520 5445 524D 5320 4F46 2054"            /* O THE TERMS OF T */
	$"4849 5320 4C49 4345 4E53 452C 2044 4F20"            /* HIS LICENSE, DO  */
	$"4E4F 5420 5553 4520 5448 4520 534F 4654"            /* NOT USE THE SOFT */
	$"5741 5245 2ECA 0A0A 494D 504F 5254 414E"            /* WARE.ÊÂÂIMPORTAN */
	$"5420 4E4F 5445 3A20 5468 6973 2073 6F66"            /* T NOTE: This sof */
	$"7477 6172 6520 6D61 7920 6265 2075 7365"            /* tware may be use */
	$"6420 746F 2072 6570 726F 6475 6365 2C20"            /* d to reproduce,  */
	$"6D6F 6469 6679 2C20 7075 626C 6973 6820"            /* modify, publish  */
	$"616E 6420 6469 7374 7269 6275 7465 206D"            /* and distribute m */
	$"6174 6572 6961 6C73 2E20 4974 2069 7320"            /* aterials. It is  */
	$"6C69 6365 6E73 6564 2074 6F20 596F 7520"            /* licensed to You  */
	$"6F6E 6C79 2066 6F72 2072 6570 726F 6475"            /* only for reprodu */
	$"6374 696F 6E2C 206D 6F64 6966 6963 6174"            /* ction, modificat */
	$"696F 6E2C 2070 7562 6C69 6361 7469 6F6E"            /* ion, publication */
	$"2061 6E64 2064 6973 7472 6962 7574 696F"            /*  and distributio */
	$"6E20 6F66 206E 6F6E 2D63 6F70 7972 6967"            /* n of non-copyrig */
	$"6874 6564 206D 6174 6572 6961 6C73 2C20"            /* hted materials,  */
	$"6D61 7465 7269 616C 7320 696E 2077 6869"            /* materials in whi */
	$"6368 2059 6F75 206F 776E 2074 6865 2063"            /* ch You own the c */
	$"6F70 7972 6967 6874 2C20 6F72 206D 6174"            /* opyright, or mat */
	$"6572 6961 6C73 2059 6F75 2061 7265 2061"            /* erials You are a */
	$"7574 686F 7269 7A65 6420 6F72 206C 6567"            /* uthorized or leg */
	$"616C 6C79 2070 6572 6D69 7474 6564 2074"            /* ally permitted t */
	$"6F20 7265 7072 6F64 7563 652C 206D 6F64"            /* o reproduce, mod */
	$"6966 792C 2070 7562 6C69 7368 2061 6E64"            /* ify, publish and */
	$"2064 6973 7472 6962 7574 652E 2049 6620"            /*  distribute. If  */
	$"596F 7520 6172 6520 756E 6365 7274 6169"            /* You are uncertai */
	$"6E20 6162 6F75 7420 596F 7572 2072 6967"            /* n about Your rig */
	$"6874 2074 6F20 636F 7079 2C20 6D6F 6469"            /* ht to copy, modi */
	$"6679 2C20 7075 626C 6973 6820 616E 6420"            /* fy, publish and  */
	$"6469 7374 7269 6275 7465 2061 6E79 206D"            /* distribute any m */
	$"6174 6572 6961 6C20 596F 7520 7368 6F75"            /* aterial You shou */
	$"6C64 2063 6F6E 7461 6374 2059 6F75 7220"            /* ld contact Your  */
	$"6C65 6761 6C20 6164 7669 736F 722E 0A0A"            /* legal advisor.ÂÂ */
	$"0A31 2E20 4765 6E65 7261 6C2E CA0A 0A41"            /* Â1. General.ÊÂÂA */
	$"2E20 5363 6F70 6520 6F66 204C 6963 656E"            /* . Scope of Licen */
	$"7365 3A20 5468 6520 4465 7665 6C6F 7065"            /* se: The Develope */
	$"7220 536F 6674 7761 7265 2028 6465 6669"            /* r Software (defi */
	$"6E65 6420 6265 6C6F 7729 2069 7320 6C69"            /* ned below) is li */
	$"6365 6E73 6564 2C20 6E6F 7420 736F 6C64"            /* censed, not sold */
	$"2C20 746F 2059 6F75 2062 7920 414E 5343"            /* , to You by ANSC */
	$"4120 496E 632E 2028 2241 4E53 4341 2229"            /* A Inc. ("ANSCA") */
	$"2061 6E64 2F6F 7220 414E 5343 4127 7320"            /*  and/or ANSCA's  */
	$"6C69 6365 6E73 6F72 732E 2054 6865 2072"            /* licensors. The r */
	$"6967 6874 7320 6772 616E 7465 6420 6865"            /* ights granted he */
	$"7265 696E 2061 7265 206C 696D 6974 6564"            /* rein are limited */
	$"2074 6F20 414E 5343 4127 7320 616E 642F"            /*  to ANSCA's and/ */
	$"6F72 2041 4E53 4341 2773 206C 6963 656E"            /* or ANSCA's licen */
	$"736F 7273 2720 7265 7370 6563 7469 7665"            /* sors' respective */
	$"2069 6E74 656C 6C65 6374 7561 6C20 7072"            /*  intellectual pr */
	$"6F70 6572 7479 2072 6967 6874 7320 696E"            /* operty rights in */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 6520 616E 6420 646F 206E"            /* oftware and do n */
	$"6F74 2069 6E63 6C75 6465 2061 6E79 206F"            /* ot include any o */
	$"7468 6572 2070 6174 656E 7473 206F 7220"            /* ther patents or  */
	$"696E 7465 6C6C 6563 7475 616C 2070 726F"            /* intellectual pro */
	$"7065 7274 7920 7269 6768 7473 2E20 596F"            /* perty rights. Yo */
	$"7520 6F77 6E20 7468 6520 6D65 6469 6120"            /* u own the media  */
	$"6F6E 2077 6869 6368 2074 6865 2044 6576"            /* on which the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"6973 2072 6563 6F72 6465 6420 6275 7420"            /* is recorded but  */
	$"414E 5343 4120 616E 642F 6F72 2041 4E53"            /* ANSCA and/or ANS */
	$"4341 2773 206C 6963 656E 736F 7228 7329"            /* CA's licensor(s) */
	$"2072 6574 6169 6E20 6F77 6E65 7273 6869"            /*  retain ownershi */
	$"7020 6F66 2074 6865 6972 2072 6573 7065"            /* p of their respe */
	$"6374 6976 6520 706F 7274 696F 6E73 206F"            /* ctive portions o */
	$"6620 7468 6520 4465 7665 6C6F 7065 7220"            /* f the Developer  */
	$"536F 6674 7761 7265 2069 7473 656C 662E"            /* Software itself. */
	$"2054 6865 2074 6572 6D73 206F 6620 7468"            /*  The terms of th */
	$"6973 204C 6963 656E 7365 2077 696C 6C20"            /* is License will  */
	$"676F 7665 726E 2061 6E79 2073 6F66 7477"            /* govern any softw */
	$"6172 6520 7570 6772 6164 6573 2070 726F"            /* are upgrades pro */
	$"7669 6465 6420 6279 2041 4E53 4341 2074"            /* vided by ANSCA t */
	$"6861 7420 7265 706C 6163 6520 616E 642F"            /* hat replace and/ */
	$"6F72 2073 7570 706C 656D 656E 7420 7468"            /* or supplement th */
	$"6520 6F72 6967 696E 616C 2044 6576 656C"            /* e original Devel */
	$"6F70 6572 2053 6F66 7477 6172 652C 2075"            /* oper Software, u */
	$"6E6C 6573 7320 7375 6368 2075 7067 7261"            /* nless such upgra */
	$"6465 2069 7320 6163 636F 6D70 616E 6965"            /* de is accompanie */
	$"6420 6279 2061 2073 6570 6172 6174 6520"            /* d by a separate  */
	$"6C69 6365 6E73 6520 696E 2077 6869 6368"            /* license in which */
	$"2063 6173 6520 7468 6520 7465 726D 7320"            /*  case the terms  */
	$"6F66 2074 6861 7420 6C69 6365 6E73 6520"            /* of that license  */
	$"7769 6C6C 2067 6F76 6572 6E2E 2041 4C4C"            /* will govern. ALL */
	$"2052 4947 4854 5320 4E4F 5420 4558 5052"            /*  RIGHTS NOT EXPR */
	$"4553 534C 5920 4752 414E 5445 4420 4845"            /* ESSLY GRANTED HE */
	$"5245 554E 4445 5220 4152 4520 5245 5345"            /* REUNDER ARE RESE */
	$"5256 4544 2054 4F20 414E 5343 412E 0A0A"            /* RVED TO ANSCA.ÂÂ */
	$"422E 2043 6F6E 7365 6E74 2074 6F20 436F"            /* B. Consent to Co */
	$"6C6C 6563 7469 6F6E 2061 6E64 2055 7365"            /* llection and Use */
	$"206F 6620 4E6F 6E2D 5065 7273 6F6E 616C"            /*  of Non-Personal */
	$"2044 6174 613A 2059 6F75 2061 6772 6565"            /*  Data: You agree */
	$"2074 6861 7420 414E 5343 4120 6D61 7920"            /*  that ANSCA may  */
	$"636F 6C6C 6563 7420 616E 6420 7573 6520"            /* collect and use  */
	$"7465 6368 6E69 6361 6C20 6461 7461 2061"            /* technical data a */
	$"6E64 2072 656C 6174 6564 2069 6E66 6F72"            /* nd related infor */
	$"6D61 7469 6F6E 2C20 696E 636C 7564 696E"            /* mation, includin */
	$"6720 6275 7420 6E6F 7420 6C69 6D69 7465"            /* g but not limite */
	$"6420 746F 2074 6563 686E 6963 616C 2061"            /* d to technical a */
	$"6E64 2F6F 7220 7374 6174 6973 7469 6361"            /* nd/or statistica */
	$"6C20 696E 666F 726D 6174 696F 6E20 6162"            /* l information ab */
	$"6F75 7420 596F 7572 2064 6576 6963 652C"            /* out Your device, */
	$"2073 7973 7465 6D20 616E 6420 6170 706C"            /*  system and appl */
	$"6963 6174 696F 6E20 736F 6674 7761 7265"            /* ication software */
	$"2C20 616E 6420 7065 7269 7068 6572 616C"            /* , and peripheral */
	$"732C 2074 6861 7420 6973 2067 6174 6865"            /* s, that is gathe */
	$"7265 6420 7065 7269 6F64 6963 616C 6C79"            /* red periodically */
	$"2074 6F20 6661 6369 6C69 7461 7465 2074"            /*  to facilitate t */
	$"6865 2070 726F 7669 7369 6F6E 206F 6620"            /* he provision of  */
	$"736F 6674 7761 7265 2075 7064 6174 6573"            /* software updates */
	$"2C20 7072 6F64 7563 7420 7375 7070 6F72"            /* , product suppor */
	$"7420 616E 6420 6F74 6865 7220 7365 7276"            /* t and other serv */
	$"6963 6573 2074 6F20 596F 7520 2869 6620"            /* ices to You (if  */
	$"616E 7929 2072 656C 6174 6564 2074 6F20"            /* any) related to  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2C20 616E 6420 746F 2076"            /* ftware, and to v */
	$"6572 6966 7920 636F 6D70 6C69 616E 6365"            /* erify compliance */
	$"2077 6974 6820 7468 6520 7465 726D 7320"            /*  with the terms  */
	$"6F66 2074 6869 7320 4167 7265 656D 656E"            /* of this Agreemen */
	$"742E 2041 4E53 4341 206D 6179 2075 7365"            /* t. ANSCA may use */
	$"2074 6869 7320 696E 666F 726D 6174 696F"            /*  this informatio */
	$"6E2C 2061 7320 6C6F 6E67 2061 7320 6974"            /* n, as long as it */
	$"2069 7320 696E 2061 2066 6F72 6D20 7468"            /*  is in a form th */
	$"6174 2064 6F65 7320 6E6F 7420 7065 7273"            /* at does not pers */
	$"6F6E 616C 6C79 2069 6465 6E74 6966 7920"            /* onally identify  */
	$"596F 752C 2074 6F20 696D 7072 6F76 6520"            /* You, to improve  */
	$"6974 7320 7072 6F64 7563 7473 206F 7220"            /* its products or  */
	$"746F 2070 726F 7669 6465 2073 6572 7669"            /* to provide servi */
	$"6365 7320 6F72 2074 6563 686E 6F6C 6F67"            /* ces or technolog */
	$"6965 7320 746F 2059 6F75 2E0A 0A43 2E20"            /* ies to You.ÂÂC.  */
	$"4163 6365 7074 616E 6365 2E20 496E 206F"            /* Acceptance. In o */
	$"7264 6572 2074 6F20 7573 6520 7468 6520"            /* rder to use the  */
	$"4465 7665 6C6F 7065 7220 536F 6674 7761"            /* Developer Softwa */
	$"7265 2C20 596F 7520 6D75 7374 2066 6972"            /* re, You must fir */
	$"7374 2061 6772 6565 2074 6F20 7468 6973"            /* st agree to this */
	$"204C 6963 656E 7365 2ECA 2049 6620 596F"            /*  License.Ê If Yo */
	$"7520 646F 206E 6F74 206F 7220 6361 6E6E"            /* u do not or cann */
	$"6F74 2061 6772 6565 2074 6F20 7468 6973"            /* ot agree to this */
	$"204C 6963 656E 7365 2C20 596F 7520 6172"            /*  License, You ar */
	$"6520 6E6F 7420 7065 726D 6974 7465 6420"            /* e not permitted  */
	$"746F 2075 7365 2074 6865 2044 6576 656C"            /* to use the Devel */
	$"6F70 6572 2053 6F66 7477 6172 652E 2044"            /* oper Software. D */
	$"6F20 6E6F 7420 646F 776E 6C6F 6164 206F"            /* o not download o */
	$"7220 7573 6520 7468 6520 4465 7665 6C6F"            /* r use the Develo */
	$"7065 7220 536F 6674 7761 7265 2069 6E20"            /* per Software in  */
	$"7468 6174 2063 6173 652E 2059 6F75 2061"            /* that case. You a */
	$"6363 6570 7420 616E 6420 6167 7265 6520"            /* ccept and agree  */
	$"746F 2074 6865 2074 6572 6D73 206F 6620"            /* to the terms of  */
	$"7468 6973 204C 6963 656E 7365 206F 6E20"            /* this License on  */
	$"596F 7572 206F 776E 2062 6568 616C 6620"            /* Your own behalf  */
	$"616E 642F 6F72 206F 6E20 6265 6861 6C66"            /* and/or on behalf */
	$"206F 6620 596F 7572 2063 6F6D 7061 6E79"            /*  of Your company */
	$"2C20 6F72 6761 6E69 7A61 7469 6F6E 206F"            /* , organization o */
	$"7220 6564 7563 6174 696F 6E61 6C20 696E"            /* r educational in */
	$"7374 6974 7574 696F 6E20 6173 2069 7473"            /* stitution as its */
	$"2061 7574 686F 7269 7A65 6420 6C65 6761"            /*  authorized lega */
	$"6C20 7265 7072 6573 656E 7461 7469 7665"            /* l representative */
	$"2C20 6279 2064 6F69 6E67 2065 6974 6865"            /* , by doing eithe */
	$"7220 6F66 2074 6865 2066 6F6C 6C6F 7769"            /* r of the followi */
	$"6E67 3ACA 0A28 6129 2063 6865 636B 696E"            /* ng:ÊÂ(a) checkin */
	$"6720 7468 6520 626F 7820 6469 7370 6C61"            /* g the box displa */
	$"7965 6420 6174 2074 6865 2065 6E64 206F"            /* yed at the end o */
	$"6620 7468 6973 2041 6772 6565 6D65 6E74"            /* f this Agreement */
	$"2069 6620 596F 7520 6172 6520 7265 6164"            /*  if You are read */
	$"696E 6720 7468 6973 206F 6E20 616E 2041"            /* ing this on an A */
	$"4E53 4341 2077 6562 7369 7465 3B20 6F72"            /* NSCA website; or */
	$"CA0A 2862 2920 636C 6963 6B69 6E67 2061"            /* ÊÂ(b) clicking a */
	$"6E20 2241 6772 6565 2220 6F72 2073 696D"            /* n "Agree" or sim */
	$"696C 6172 2062 7574 746F 6E2C 2077 6865"            /* ilar button, whe */
	$"7265 2074 6869 7320 6F70 7469 6F6E 2069"            /* re this option i */
	$"7320 7072 6F76 6964 6564 2062 7920 414E"            /* s provided by AN */
	$"5343 412E CA0A 0A44 2E20 4465 6669 6E69"            /* SCA.ÊÂÂD. Defini */
	$"7469 6F6E 732E CA0A 0A22 4465 7665 6C6F"            /* tions.ÊÂÂ"Develo */
	$"7065 7220 536F 6674 7761 7265 2220 636F"            /* per Software" co */
	$"6C6C 6563 7469 7665 6C79 206D 6561 6E73"            /* llectively means */
	$"3A20 2861 2920 7468 6520 5344 4B20 2864"            /* : (a) the SDK (d */
	$"6566 696E 6564 2062 656C 6F77 2920 616E"            /* efined below) an */
	$"6420 2862 2920 7468 6520 4170 706C 6963"            /* d (b) the Applic */
	$"6174 696F 6E73 2028 6465 6669 6E65 6420"            /* ations (defined  */
	$"6265 6C6F 7729 2063 7265 6174 6564 2061"            /* below) created a */
	$"6E64 2070 726F 6475 6365 6420 6279 2074"            /* nd produced by t */
	$"6865 2053 444B 2ECA 0A0A 2241 7070 6C69"            /* he SDK.ÊÂÂ"Appli */
	$"6361 7469 6F6E 2220 6D65 616E 7320 7468"            /* cation" means th */
	$"6520 736F 6674 7761 7265 2070 726F 6772"            /* e software progr */
	$"616D 2074 6861 7420 7275 6E73 206F 6E20"            /* am that runs on  */
	$"616E 2041 4E53 4341 2D73 7570 706F 7274"            /* an ANSCA-support */
	$"6564 2064 6576 6963 6520 616E 6420 7468"            /* ed device and th */
	$"6174 2069 7320 7072 6F64 7563 6564 2062"            /* at is produced b */
	$"7920 7468 6520 5344 4B20 7573 696E 6720"            /* y the SDK using  */
	$"736F 7572 6365 2063 6F64 652C 2069 6D61"            /* source code, ima */
	$"6765 732C 2073 6F75 6E64 732C 2061 6E64"            /* ges, sounds, and */
	$"206F 7468 6572 206D 6564 6961 2070 726F"            /*  other media pro */
	$"6475 6365 6420 6279 2059 6F75 2069 6E20"            /* duced by You in  */
	$"636F 6D70 6C69 616E 6365 2077 6974 6820"            /* compliance with  */
	$"7468 6520 446F 6375 6D65 6E74 6174 696F"            /* the Documentatio */
	$"6E20 2864 6566 696E 6564 2062 656C 6F77"            /* n (defined below */
	$"2920 616E 6420 7468 6520 7465 726D 7320"            /* ) and the terms  */
	$"6F66 2074 6869 7320 4C69 6365 6E73 652C"            /* of this License, */
	$"2075 6E64 6572 2059 6F75 7220 6F77 6E20"            /*  under Your own  */
	$"7472 6164 656D 6172 6B20 6F72 2062 7261"            /* trademark or bra */
	$"6E64 2C20 616E 6420 666F 7220 7370 6563"            /* nd, and for spec */
	$"6966 6963 2075 7365 2077 6974 6820 414E"            /* ific use with AN */
	$"5343 412D 7375 7070 6F72 7465 6420 6465"            /* SCA-supported de */
	$"7669 6365 732C 2069 6E63 6C75 6469 6E67"            /* vices, including */
	$"2062 7567 2066 6978 6573 2C20 7570 6772"            /*  bug fixes, upgr */
	$"6164 6573 2C20 6D6F 6469 6669 6361 7469"            /* ades, modificati */
	$"6F6E 732C 2065 6E68 616E 6365 6D65 6E74"            /* ons, enhancement */
	$"732C 2073 7570 706C 656D 656E 7473 2074"            /* s, supplements t */
	$"6F2C 2072 6576 6973 696F 6E73 2C20 6E65"            /* o, revisions, ne */
	$"7720 7265 6C65 6173 6573 2061 6E64 206E"            /* w releases and n */
	$"6577 2076 6572 7369 6F6E 7320 6F66 2073"            /* ew versions of s */
	$"7563 6820 736F 6674 7761 7265 2070 726F"            /* uch software pro */
	$"6772 616D 732E 0A0A 2241 7574 686F 7269"            /* grams.ÂÂ"Authori */
	$"7A65 6420 4465 7665 6C6F 7065 7273 2220"            /* zed Developers"  */
	$"6D65 616E 7320 596F 7572 2065 6D70 6C6F"            /* means Your emplo */
	$"7965 6573 2061 6E64 2063 6F6E 7472 6163"            /* yees and contrac */
	$"746F 7273 2C20 6D65 6D62 6572 7320 6F66"            /* tors, members of */
	$"2059 6F75 7220 6F72 6761 6E69 7A61 7469"            /*  Your organizati */
	$"6F6E 206F 722C 2069 6620 596F 7520 6172"            /* on or, if You ar */
	$"6520 616E 2065 6475 6361 7469 6F6E 616C"            /* e an educational */
	$"2069 6E73 7469 7475 7469 6F6E 2C20 596F"            /*  institution, Yo */
	$"7572 2066 6163 756C 7479 2061 6E64 2073"            /* ur faculty and s */
	$"7461 6666 2077 686F 2028 6129 2065 6163"            /* taff who (a) eac */
	$"6820 6861 7665 2061 206C 6963 656E 7365"            /* h have a license */
	$"2074 6F20 7573 6520 7468 6520 5344 4B2C"            /*  to use the SDK, */
	$"2028 6229 2065 6163 6820 6861 7665 2061"            /*  (b) each have a */
	$"6E20 6163 7469 7665 2061 6E64 2076 616C"            /* n active and val */
	$"6964 2061 6363 6F75 6E74 2077 6974 6820"            /* id account with  */
	$"414E 5343 412C 2028 6329 2068 6176 6520"            /* ANSCA, (c) have  */
	$"6120 6465 6D6F 6E73 7472 6162 6C65 206E"            /* a demonstrable n */
	$"6565 6420 746F 206B 6E6F 7720 6F72 2075"            /* eed to know or u */
	$"7365 2074 6865 2053 444B 2069 6E20 6F72"            /* se the SDK in or */
	$"6465 7220 746F 2064 6576 656C 6F70 2061"            /* der to develop a */
	$"6E64 2074 6573 7420 4170 706C 6963 6174"            /* nd test Applicat */
	$"696F 6E73 2C20 616E 6420 2864 2920 746F"            /* ions, and (d) to */
	$"2074 6865 2065 7874 656E 7420 7375 6368"            /*  the extent such */
	$"2069 6E64 6976 6964 7561 6C73 2077 696C"            /*  individuals wil */
	$"6C20 6861 7665 2061 6363 6573 7320 746F"            /* l have access to */
	$"2041 4E53 4341 2043 6F6E 6669 6465 6E74"            /*  ANSCA Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E2C"            /* ial Information, */
	$"2065 6163 6820 6861 7665 2077 7269 7474"            /*  each have writt */
	$"656E 2061 6E64 2062 696E 6469 6E67 2061"            /* en and binding a */
	$"6772 6565 6D65 6E74 7320 7769 7468 2059"            /* greements with Y */
	$"6F75 2074 6F20 7072 6F74 6563 7420 7468"            /* ou to protect th */
	$"6520 756E 6175 7468 6F72 697A 6564 2075"            /* e unauthorized u */
	$"7365 2061 6E64 2064 6973 636C 6F73 7572"            /* se and disclosur */
	$"6520 6F66 2073 7563 6820 414E 5343 4120"            /* e of such ANSCA  */
	$"616E 6420 7468 6972 6420 7061 7274 7920"            /* and third party  */
	$"636F 6E66 6964 656E 7469 616C 2069 6E66"            /* confidential inf */
	$"6F72 6D61 7469 6F6E 2E0A 0A22 5344 4B22"            /* ormation.ÂÂ"SDK" */
	$"2028 536F 6674 7761 7265 2044 6576 656C"            /*  (Software Devel */
	$"6F70 6D65 6E74 204B 6974 2920 6D65 616E"            /* opment Kit) mean */
	$"7320 7468 6520 446F 6375 6D65 6E74 6174"            /* s the Documentat */
	$"696F 6E2C 2073 6F66 7477 6172 6520 2873"            /* ion, software (s */
	$"6F75 7263 6520 636F 6465 2061 6E64 206F"            /* ource code and o */
	$"626A 6563 7420 636F 6465 292C 2041 7070"            /* bject code), App */
	$"6C69 6361 7469 6F6E 732C 2073 616D 706C"            /* lications, sampl */
	$"6520 6F72 2065 7861 6D70 6C65 2063 6F64"            /* e or example cod */
	$"652C 2073 696D 756C 6174 6F72 2C20 746F"            /* e, simulator, to */
	$"6F6C 732C 2075 7469 6C69 7469 6573 2C20"            /* ols, utilities,  */
	$"6C69 6272 6172 6965 732C 2041 5049 732C"            /* libraries, APIs, */
	$"2064 6174 612C 2066 696C 6573 2C20 616E"            /*  data, files, an */
	$"6420 6F74 6865 7220 6D61 7465 7269 616C"            /* d other material */
	$"7320 7072 6F76 6964 6564 206F 7220 6D61"            /* s provided or ma */
	$"6465 2061 7661 696C 6162 6C65 2062 7920"            /* de available by  */
	$"414E 5343 4120 666F 7220 7573 6520 6279"            /* ANSCA for use by */
	$"2059 6F75 2069 6E20 636F 6E6E 6563 7469"            /*  You in connecti */
	$"6F6E 2077 6974 6820 596F 7572 2041 7070"            /* on with Your App */
	$"6C69 6361 7469 6F6E 2064 6576 656C 6F70"            /* lication develop */
	$"6D65 6E74 2077 6865 7468 6572 206F 6E20"            /* ment whether on  */
	$"6469 736B 2C20 6F6E 2070 7269 6E74 206F"            /* disk, on print o */
	$"7220 656C 6563 7472 6F6E 6963 2064 6F63"            /* r electronic doc */
	$"756D 656E 7461 7469 6F6E 2C20 696E 2072"            /* umentation, in r */
	$"6561 6420 6F6E 6C79 206D 656D 6F72 792C"            /* ead only memory, */
	$"206F 6E20 616E 7920 6F74 6865 7220 6D65"            /*  on any other me */
	$"6469 612C 206F 7220 6D61 6465 2061 7661"            /* dia, or made ava */
	$"696C 6162 6C65 206F 6E20 6120 7365 7276"            /* ilable on a serv */
	$"6572 2E0A 0A22 446F 6375 6D65 6E74 6564"            /* er.ÂÂ"Documented */
	$"2041 5049 2873 2922 206D 6561 6E73 2074"            /*  API(s)" means t */
	$"6865 2041 7070 6C69 6361 7469 6F6E 2050"            /* he Application P */
	$"726F 6772 616D 6D69 6E67 2049 6E74 6572"            /* rogramming Inter */
	$"6661 6365 2873 2920 646F 6375 6D65 6E74"            /* face(s) document */
	$"6564 2062 7920 414E 5343 4120 696E 2070"            /* ed by ANSCA in p */
	$"7562 6C69 7368 6564 2041 4E53 4341 2044"            /* ublished ANSCA D */
	$"6F63 756D 656E 7461 7469 6F6E 2061 6E64"            /* ocumentation and */
	$"2077 6869 6368 2061 7265 2063 6F6E 7461"            /*  which are conta */
	$"696E 6564 2069 6E20 7468 6520 5344 4B2E"            /* ined in the SDK. */
	$"0A0A 2244 6F63 756D 656E 7461 7469 6F6E"            /* ÂÂ"Documentation */
	$"2220 6D65 616E 7320 616E 7920 7465 6368"            /* " means any tech */
	$"6E69 6361 6C20 6F72 206F 7468 6572 2073"            /* nical or other s */
	$"7065 6369 6669 6361 7469 6F6E 7320 6F72"            /* pecifications or */
	$"2064 6F63 756D 656E 7461 7469 6F6E 2074"            /*  documentation t */
	$"6861 7420 414E 5343 4120 6D61 7920 6D61"            /* hat ANSCA may ma */
	$"6B65 2061 7661 696C 6162 6C65 206F 7220"            /* ke available or  */
	$"7072 6F76 6964 6520 746F 2059 6F75 2072"            /* provide to You r */
	$"656C 6174 696E 6720 746F 206F 7220 666F"            /* elating to or fo */
	$"7220 7573 6520 696E CA0A 636F 6E6E 6563"            /* r use inÊÂconnec */
	$"7469 6F6E 2077 6974 6820 7468 6520 4465"            /* tion with the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2E0A 0A22 4C69 6365 6E73 6522 2C20 224C"            /* .ÂÂ"License", "L */
	$"6963 656E 7365 2041 6772 6565 6D65 6E74"            /* icense Agreement */
	$"222C 2061 6E64 2022 4167 7265 656D 656E"            /* ", and "Agreemen */
	$"7422 206D 6561 6E73 2061 6E64 2072 6566"            /* t" means and ref */
	$"6572 7320 746F 2074 6869 7320 536F 6674"            /* ers to this Soft */
	$"7761 7265 204C 6963 656E 7365 2041 6772"            /* ware License Agr */
	$"6565 6D65 6E74 2E0A 0A22 4175 7468 6F72"            /* eement.ÂÂ"Author */
	$"697A 6564 2043 6F6D 7075 7465 7222 206D"            /* ized Computer" m */
	$"6561 6E73 2061 2063 6F6D 7075 7465 7220"            /* eans a computer  */
	$"7768 6963 6820 6861 7320 6265 656E 2061"            /* which has been a */
	$"7574 686F 7269 7A65 6420 6279 2041 4E53"            /* uthorized by ANS */
	$"4341 2073 6572 7665 7273 2074 6F20 7275"            /* CA servers to ru */
	$"6E20 7468 6520 4465 7665 6C6F 7065 7220"            /* n the Developer  */
	$"536F 6674 7761 7265 2E0A 0A22 5570 6461"            /* Software.ÂÂ"Upda */
	$"7465 7322 206D 6561 6E73 2062 7567 2066"            /* tes" means bug f */
	$"6978 6573 2C20 7570 6461 7465 732C 2075"            /* ixes, updates, u */
	$"7067 7261 6465 732C 206D 6F64 6966 6963"            /* pgrades, modific */
	$"6174 696F 6E73 2C20 656E 6861 6E63 656D"            /* ations, enhancem */
	$"656E 7473 2C20 7375 7070 6C65 6D65 6E74"            /* ents, supplement */
	$"732C 2061 6E64 206E 6577 2072 656C 6561"            /* s, and new relea */
	$"7365 7320 6F72 2076 6572 7369 6F6E 7320"            /* ses or versions  */
	$"6F66 2074 6865 2044 6576 656C 6F70 6572"            /* of the Developer */
	$"2053 6F66 7477 6172 652C 206F 7220 746F"            /*  Software, or to */
	$"2061 6E79 2070 6172 7420 6F66 2074 6865"            /*  any part of the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652E 0A0A 2259 6F75 222C 2022 596F"            /* are.ÂÂ"You", "Yo */
	$"7572 2220 616E 6420 224C 6963 656E 7365"            /* ur" and "License */
	$"6522 206D 6561 6E73 2061 6E64 2072 6566"            /* e" means and ref */
	$"6572 7320 746F 2074 6865 2070 6572 736F"            /* ers to the perso */
	$"6E28 7329 206F 7220 6C65 6761 6C20 656E"            /* n(s) or legal en */
	$"7469 7479 2075 7369 6E67 2074 6865 2044"            /* tity using the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6F72 206F 7468 6572 7769 7365 2065"            /* e or otherwise e */
	$"7865 7263 6973 696E 6720 7269 6768 7473"            /* xercising rights */
	$"2075 6E64 6572 2074 6869 7320 4167 7265"            /*  under this Agre */
	$"656D 656E 742E CA20 4966 2059 6F75 2061"            /* ement.Ê If You a */
	$"7265 2065 6E74 6572 696E 6720 696E 746F"            /* re entering into */
	$"2074 6869 7320 4167 7265 656D 656E 7420"            /*  this Agreement  */
	$"6F6E 2062 6568 616C 6620 6F66 2059 6F75"            /* on behalf of You */
	$"7220 636F 6D70 616E 792C 206F 7267 616E"            /* r company, organ */
	$"697A 6174 696F 6E20 6F72 2065 6475 6361"            /* ization or educa */
	$"7469 6F6E 616C 2069 6E73 7469 7475 7469"            /* tional instituti */
	$"6F6E 2C20 2259 6F75 2220 6F72 2022 596F"            /* on, "You" or "Yo */
	$"7572 2220 7265 6665 7273 2074 6F20 596F"            /* ur" refers to Yo */
	$"7572 2063 6F6D 7061 6E79 2C20 6F72 6761"            /* ur company, orga */
	$"6E69 7A61 7469 6F6E 206F 7220 6564 7563"            /* nization or educ */
	$"6174 696F 6E61 6C20 696E 7374 6974 7574"            /* ational institut */
	$"696F 6E20 6173 2077 656C 6C2E 0A0A 0A32"            /* ion as well.ÂÂÂ2 */
	$"2E20 5065 726D 6974 7465 6420 4C69 6365"            /* . Permitted Lice */
	$"6E73 6520 5573 6573 2061 6E64 2052 6573"            /* nse Uses and Res */
	$"7472 6963 7469 6F6E 732E 20CA 0A0A 412E"            /* trictions. ÊÂÂA. */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652E 200A 2861 2920 5468 6520 5344"            /* are. Â(a) The SD */
	$"4B20 6973 206C 6963 656E 7365 6420 6F6E"            /* K is licensed on */
	$"2061 2070 6572 2073 7562 7363 7269 7074"            /*  a per subscript */
	$"696F 6E20 6261 7369 732E 2057 6869 6C65"            /* ion basis. While */
	$"2079 6F75 7220 7375 6273 6372 6970 7469"            /*  your subscripti */
	$"6F6E 2069 7320 6163 7469 7665 2C20 796F"            /* on is active, yo */
	$"7520 6D61 793A 200A 2A20 696E 7374 616C"            /* u may: Â* instal */
	$"6C20 6F6E 6520 636F 7079 206F 6620 7468"            /* l one copy of th */
	$"6520 5344 4B20 6F6E 206F 6E65 2041 7574"            /* e SDK on one Aut */
	$"686F 7269 7A65 6420 436F 6D70 7574 6572"            /* horized Computer */
	$"2E20 0A2A 2075 7365 206F 6E65 2063 6F70"            /* . Â* use one cop */
	$"7920 6F66 2074 6865 2053 444B 206F 6E20"            /* y of the SDK on  */
	$"7468 6520 4175 7468 6F72 697A 6564 2043"            /* the Authorized C */
	$"6F6D 7075 7465 7220 6174 2061 2074 696D"            /* omputer at a tim */
	$"652E 200A 2A20 696E 7374 616C 6C20 616E"            /* e. Â* install an */
	$"6F74 6865 7220 636F 7079 206F 6620 7468"            /* other copy of th */
	$"6520 5344 4B20 6F6E 2061 2070 6F72 7461"            /* e SDK on a porta */
	$"626C 6520 6465 7669 6365 2061 7574 686F"            /* ble device autho */
	$"7269 7A65 6420 6279 2041 4E53 4341 2773"            /* rized by ANSCA's */
	$"2073 6572 7665 7220 666F 7220 7573 6520"            /*  server for use  */
	$"6279 2074 6865 2073 696E 676C 6520 7072"            /* by the single pr */
	$"696D 6172 7920 7573 6572 206F 6620 7468"            /* imary user of th */
	$"6520 4175 7468 6F72 697A 6564 2043 6F6D"            /* e Authorized Com */
	$"7075 7465 722E 200A 2862 2920 5375 626A"            /* puter. Â(b) Subj */
	$"6563 7420 746F 2074 6865 2074 6572 6D73"            /* ect to the terms */
	$"2062 656C 6F77 2C20 414E 5343 4120 6772"            /*  below, ANSCA gr */
	$"616E 7473 2059 6F75 2061 206E 6F6E 2D65"            /* ants You a non-e */
	$"7863 6C75 7369 7665 2C20 6E6F 6E2D 7472"            /* xclusive, non-tr */
	$"616E 7366 6572 6162 6C65 206C 6963 656E"            /* ansferable licen */
	$"7365 2028 7769 7468 6F75 7420 7468 6520"            /* se (without the  */
	$"7269 6768 7420 746F 2073 7562 6C69 6365"            /* right to sublice */
	$"6E73 6529 2028 6929 2074 6F20 7573 6520"            /* nse) (i) to use  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2073 6F6C 656C 7920 666F"            /* ftware solely fo */
	$"7220 7468 6520 7075 7270 6F73 6520 6F66"            /* r the purpose of */
	$"2064 6576 656C 6F70 696E 6720 4170 706C"            /*  developing Appl */
	$"6963 6174 696F 6E73 2066 6F72 206D 6F62"            /* ications for mob */
	$"696C 6520 7068 6F6E 6573 2061 6E64 206F"            /* ile phones and o */
	$"7468 6572 2073 6F66 7477 6172 6520 666F"            /* ther software fo */
	$"7220 6D6F 6269 6C65 2070 686F 6E65 733B"            /* r mobile phones; */
	$"2028 6969 2920 746F 2063 6F70 7920 5344"            /*  (ii) to copy SD */
	$"4B20 666F 7220 6172 6368 6976 616C 206F"            /* K for archival o */
	$"7220 6261 636B 7570 2070 7572 706F 7365"            /* r backup purpose */
	$"732C 2070 726F 7669 6465 6420 7468 6174"            /* s, provided that */
	$"2061 6C6C 2074 6974 6C65 7320 616E 6420"            /*  all titles and  */
	$"7472 6164 656D 6172 6B73 2C20 636F 7079"            /* trademarks, copy */
	$"7269 6768 742C 2061 6E64 2072 6573 7472"            /* right, and restr */
	$"6963 7465 6420 7269 6768 7473 206E 6F74"            /* icted rights not */
	$"6963 6573 2061 7265 2072 6570 726F 6475"            /* ices are reprodu */
	$"6365 6420 6F6E 2073 7563 6820 636F 7069"            /* ced on such copi */
	$"6573 3B20 616E 6420 2869 6969 2920 746F"            /* es; and (iii) to */
	$"206D 6172 6B65 7420 616E 6420 6469 7374"            /*  market and dist */
	$"7269 6275 7465 2C20 6469 7265 6374 6C79"            /* ribute, directly */
	$"206F 7220 696E 6469 7265 6374 6C79 2074"            /*  or indirectly t */
	$"6872 6F75 6768 2064 6973 7472 6962 7574"            /* hrough distribut */
	$"6F72 732C 2041 7070 6C69 6361 7469 6F6E"            /* ors, Application */
	$"7320 6372 6561 7465 6420 7769 7468 2074"            /* s created with t */
	$"6865 2053 444B 2C20 7072 6F76 6964 6564"            /* he SDK, provided */
	$"2074 6861 7420 2861 2920 596F 7520 7769"            /*  that (a) You wi */
	$"6C6C 206E 6F74 2C20 616E 6420 7769 6C6C"            /* ll not, and will */
	$"206E 6F74 2070 6572 6D69 7420 616E 7920"            /*  not permit any  */
	$"7468 6972 6420 7061 7274 7920 746F 2C20"            /* third party to,  */
	$"6D6F 6469 6679 2C20 6372 6561 7465 2064"            /* modify, create d */
	$"6572 6976 6174 6976 6520 776F 726B 7320"            /* erivative works  */
	$"6F66 2C20 7472 616E 736C 6174 652C 2072"            /* of, translate, r */
	$"6576 6572 7365 2065 6E67 696E 6565 722C"            /* everse engineer, */
	$"2064 6563 6F6D 7069 6C65 2C20 6469 7361"            /*  decompile, disa */
	$"7373 656D 626C 652C 206F 7220 6F74 6865"            /* ssemble, or othe */
	$"7277 6973 6520 7265 6475 6365 2074 6F20"            /* rwise reduce to  */
	$"6875 6D61 6E20 7065 7263 6569 7661 626C"            /* human perceivabl */
	$"6520 666F 726D 2061 6E79 2070 6F72 7469"            /* e form any porti */
	$"6F6E 206F 6620 7468 6520 4170 706C 6963"            /* on of the Applic */
	$"6174 696F 6E20 6372 6561 7465 6420 6279"            /* ation created by */
	$"2074 6865 2053 444B 2061 6E64 2028 6229"            /*  the SDK and (b) */
	$"2061 6E79 2041 7070 6C69 6361 7469 6F6E"            /*  any Application */
	$"2063 7265 6174 6564 2077 6974 6820 7468"            /*  created with th */
	$"6520 5344 4B20 6973 2064 6973 7472 6962"            /* e SDK is distrib */
	$"7574 6564 2074 6F20 656E 6420 7573 6572"            /* uted to end user */
	$"7320 756E 6465 7220 616E 2065 6E66 6F72"            /* s under an enfor */
	$"6365 6162 6C65 2065 6E64 2D75 7365 7220"            /* ceable end-user  */
	$"6C69 6365 6E73 6520 6167 7265 656D 656E"            /* license agreemen */
	$"7420 636F 6E74 6169 6E69 6E67 2061 7420"            /* t containing at  */
	$"6C65 6173 7420 7468 6520 666F 6C6C 6F77"            /* least the follow */
	$"696E 6720 6D69 6E69 6D75 6D20 7465 726D"            /* ing minimum term */
	$"733A 0A2A 2050 726F 6869 6269 7469 6F6E"            /* s:Â* Prohibition */
	$"2061 6761 696E 7374 2064 6973 7472 6962"            /*  against distrib */
	$"7574 696F 6E20 616E 6420 636F 7079 696E"            /* ution and copyin */
	$"672E 0A2A 2050 726F 6869 6269 7469 6F6E"            /* g.Â* Prohibition */
	$"2061 6761 696E 7374 206D 6F64 6966 6963"            /*  against modific */
	$"6174 696F 6E73 2061 6E64 2064 6572 6976"            /* ations and deriv */
	$"6174 6976 6520 776F 726B 732E 0A2A 2050"            /* ative works.Â* P */
	$"726F 6869 6269 7469 6F6E 2061 6761 696E"            /* rohibition again */
	$"7374 2064 6563 6F6D 7069 6C69 6E67 2C20"            /* st decompiling,  */
	$"7265 7665 7273 6520 656E 6769 6E65 6572"            /* reverse engineer */
	$"696E 672C 2064 6973 6173 7365 6D62 6C69"            /* ing, disassembli */
	$"6E67 2C20 616E 6420 6F74 6865 7277 6973"            /* ng, and otherwis */
	$"6520 7265 6475 6369 6E67 2074 6865 2073"            /* e reducing the s */
	$"6F66 7477 6172 6520 746F 2061 2068 756D"            /* oftware to a hum */
	$"616E 2D70 6572 6365 6976 6162 6C65 2066"            /* an-perceivable f */
	$"6F72 6D2E 0A2A 2050 726F 7669 7369 6F6E"            /* orm.Â* Provision */
	$"2069 6E64 6963 6174 696E 6720 6F77 6E65"            /*  indicating owne */
	$"7273 6869 7020 6F66 2073 6F66 7477 6172"            /* rship of softwar */
	$"6520 6279 2059 6F75 2061 6E64 2059 6F75"            /* e by You and You */
	$"7220 7375 7070 6C69 6572 732E 0A2A 2044"            /* r suppliers.Â* D */
	$"6973 636C 6169 6D65 7220 6F66 2061 6C6C"            /* isclaimer of all */
	$"2061 7070 6C69 6361 626C 6520 7374 6174"            /*  applicable stat */
	$"7574 6F72 7920 7761 7272 616E 7469 6573"            /* utory warranties */
	$"2C20 746F 2074 6865 2066 756C 6C20 6578"            /* , to the full ex */
	$"7465 6E74 2061 6C6C 6F77 6564 2062 7920"            /* tent allowed by  */
	$"6C61 772E 0A2A 204C 696D 6974 6174 696F"            /* law.Â* Limitatio */
	$"6E20 6F66 206C 6961 6269 6C69 7479 206E"            /* n of liability n */
	$"6F74 2074 6F20 6578 6365 6564 2070 7269"            /* ot to exceed pri */
	$"6365 206F 6620 596F 7572 2050 726F 6475"            /* ce of Your Produ */
	$"6374 2C20 616E 6420 7072 6F76 6973 696F"            /* ct, and provisio */
	$"6E20 7468 6174 2073 6F6C 6520 7265 6D65"            /* n that sole reme */
	$"6479 2073 6861 6C6C 2062 6520 6120 7269"            /* dy shall be a ri */
	$"6768 7420 6F66 2072 6574 7572 6E20 616E"            /* ght of return an */
	$"6420 7265 6675 6E64 2C20 6966 2061 6E79"            /* d refund, if any */
	$"2C20 6672 6F6D 2059 6F75 2E0A 2A20 4469"            /* , from You.Â* Di */
	$"7363 6C61 696D 6572 206F 6620 696E 6469"            /* sclaimer of indi */
	$"7265 6374 2C20 7370 6563 6961 6C2C 2069"            /* rect, special, i */
	$"6E63 6964 656E 7461 6C2C 2070 756E 6974"            /* ncidental, punit */
	$"6976 652C 2061 6E64 2063 6F6E 7365 7175"            /* ive, and consequ */
	$"656E 7469 616C 2064 616D 6167 6573 2E0A"            /* ential damages.Â */
	$"2A20 4D61 6B69 6E67 2041 4E53 4341 2061"            /* * Making ANSCA a */
	$"2074 6869 7264 2D70 6172 7479 2062 656E"            /*  third-party ben */
	$"6566 6963 6961 7279 2066 6F72 2070 7572"            /* eficiary for pur */
	$"706F 7365 7320 6F66 2065 6E66 6F72 6369"            /* poses of enforci */
	$"6E67 2069 7473 2069 6E74 656C 6C65 6374"            /* ng its intellect */
	$"7561 6C20 7072 6F70 6572 7479 2072 6967"            /* ual property rig */
	$"6874 7320 6F72 2061 6E79 2064 6973 636C"            /* hts or any discl */
	$"6169 6D65 7273 206F 7220 6C69 6D69 7461"            /* aimers or limita */
	$"7469 6F6E 732E 0A0A 422E 204F 7065 6E2D"            /* tions.ÂÂB. Open- */
	$"536F 7572 6365 6420 536F 6674 7761 7265"            /* Sourced Software */
	$"2E20 5468 6520 7465 726D 7320 616E 6420"            /* . The terms and  */
	$"636F 6E64 6974 696F 6E73 206F 6620 7468"            /* conditions of th */
	$"6973 204C 6963 656E 7365 2073 6861 6C6C"            /* is License shall */
	$"206E 6F74 2061 7070 6C79 2074 6F20 616E"            /*  not apply to an */
	$"7920 4F70 656E 2053 6F75 7263 6520 536F"            /* y Open Source So */
	$"6674 7761 7265 2061 6363 6F6D 7061 6E79"            /* ftware accompany */
	$"696E 6720 7468 6520 4465 7665 6C6F 7065"            /* ing the Develope */
	$"7220 536F 6674 7761 7265 2E20 416E 7920"            /* r Software. Any  */
	$"7375 6368 204F 7065 6E20 536F 7572 6365"            /* such Open Source */
	$"2053 6F66 7477 6172 6520 6973 2070 726F"            /*  Software is pro */
	$"7669 6465 6420 756E 6465 7220 7468 6520"            /* vided under the  */
	$"7465 726D 7320 6F66 2074 6865 206F 7065"            /* terms of the ope */
	$"6E20 736F 7572 6365 206C 6963 656E 7365"            /* n source license */
	$"2061 6772 6565 6D65 6E74 206F 7220 636F"            /*  agreement or co */
	$"7079 7269 6768 7420 6E6F 7469 6365 2061"            /* pyright notice a */
	$"6363 6F6D 7061 6E79 696E 6720 7375 6368"            /* ccompanying such */
	$"204F 7065 6E20 536F 7572 6365 2053 6F66"            /*  Open Source Sof */
	$"7477 6172 6520 6F72 2069 6E20 7468 6520"            /* tware or in the  */
	$"6F70 656E 2073 6F75 7263 6520 6C69 6365"            /* open source lice */
	$"6E73 6573 2066 696C 6520 6163 636F 6D70"            /* nses file accomp */
	$"616E 7969 6E67 2074 6865 2044 6576 656C"            /* anying the Devel */
	$"6F70 6572 2053 6F66 7477 6172 652E CA0A"            /* oper Software.ÊÂ */
	$"0A43 2E20 5361 6D70 6C65 2043 6F64 652E"            /* ÂC. Sample Code. */
	$"2043 6572 7461 696E 2070 6F72 7469 6F6E"            /*  Certain portion */
	$"7320 6F66 2074 6865 2044 6576 656C 6F70"            /* s of the Develop */
	$"6572 2053 6F66 7477 6172 6520 636F 6E73"            /* er Software cons */
	$"6973 7420 6F66 2073 616D 706C 6520 6F72"            /* ist of sample or */
	$"2065 7861 6D70 6C65 2063 6F64 6520 7072"            /*  example code pr */
	$"6F76 6964 6564 2062 7920 414E 5343 4120"            /* ovided by ANSCA  */
	$"2822 5361 6D70 6C65 2043 6F64 6522 292E"            /* ("Sample Code"). */
	$"CA20 596F 7520 6D61 7920 7573 652C 2072"            /* Ê You may use, r */
	$"6570 726F 6475 6365 2C20 6D6F 6469 6679"            /* eproduce, modify */
	$"2061 6E64 2072 6564 6973 7472 6962 7574"            /*  and redistribut */
	$"6520 7375 6368 2053 616D 706C 6520 436F"            /* e such Sample Co */
	$"6465 2069 6E20 6163 636F 7264 616E 6365"            /* de in accordance */
	$"2077 6974 6820 7468 6520 6C69 6365 6E73"            /*  with the licens */
	$"696E 6720 7465 726D 7320 6163 636F 6D70"            /* ing terms accomp */
	$"616E 7969 6E67 2073 7563 6820 5361 6D70"            /* anying such Samp */
	$"6C65 2043 6F64 6520 6F72 2072 656C 6174"            /* le Code or relat */
	$"6564 2070 726F 6A65 6374 2873 292E 0A0A"            /* ed project(s).ÂÂ */
	$"442E 204E 6F20 5375 7070 6F72 7420 5365"            /* D. No Support Se */
	$"7276 6963 6573 2E20 414E 5343 4120 6973"            /* rvices. ANSCA is */
	$"2075 6E64 6572 206E 6F20 6F62 6C69 6761"            /*  under no obliga */
	$"7469 6F6E 2074 6F20 7375 7070 6F72 7420"            /* tion to support  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2069 6E20 616E 7920 7761"            /* ftware in any wa */
	$"7920 6F72 2074 6F20 7072 6F76 6964 6520"            /* y or to provide  */
	$"616E 7920 7570 6772 6164 6573 2074 6F20"            /* any upgrades to  */
	$"596F 752E 0A0A 452E 2052 6573 7472 6963"            /* You.ÂÂE. Restric */
	$"7469 6F6E 732E 2059 6F75 2073 6861 6C6C"            /* tions. You shall */
	$"206E 6F74 2063 6F70 7920 6F72 2075 7365"            /*  not copy or use */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 6520 6578 6365 7074 2061"            /* oftware except a */
	$"7320 6578 7072 6573 736C 7920 7065 726D"            /* s expressly perm */
	$"6974 7465 6420 696E 2074 6869 7320 4C69"            /* itted in this Li */
	$"6365 6E73 652E 2059 6F75 2077 696C 6C20"            /* cense. You will  */
	$"6E6F 742C 2061 6E64 2077 696C 6C20 6E6F"            /* not, and will no */
	$"7420 7065 726D 6974 2061 6E79 2074 6869"            /* t permit any thi */
	$"7264 2070 6172 7479 2074 6F2C 2073 7562"            /* rd party to, sub */
	$"6C69 6365 6E73 652C 2072 656E 742C 2063"            /* license, rent, c */
	$"6F70 792C 206D 6F64 6966 792C 2063 7265"            /* opy, modify, cre */
	$"6174 6520 6465 7269 7661 7469 7665 2077"            /* ate derivative w */
	$"6F72 6B73 206F 662C 2074 7261 6E73 6C61"            /* orks of, transla */
	$"7465 2C20 7265 7665 7273 6520 656E 6769"            /* te, reverse engi */
	$"6E65 6572 2C20 6465 636F 6D70 696C 652C"            /* neer, decompile, */
	$"2064 6973 6173 7365 6D62 6C65 2C20 6F72"            /*  disassemble, or */
	$"206F 7468 6572 7769 7365 2072 6564 7563"            /*  otherwise reduc */
	$"6520 746F 2068 756D 616E 2070 6572 6365"            /* e to human perce */
	$"6976 6162 6C65 2066 6F72 6D20 616E 7920"            /* ivable form any  */
	$"706F 7274 696F 6E20 6F66 2074 6865 2044"            /* portion of the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"652E 2054 6865 2044 6576 656C 6F70 6572"            /* e. The Developer */
	$"2053 6F66 7477 6172 6520 616E 6420 616C"            /*  Software and al */
	$"6C20 7065 7266 6F72 6D61 6E63 6520 6461"            /* l performance da */
	$"7461 2061 6E64 2074 6573 7420 7265 7375"            /* ta and test resu */
	$"6C74 732C 2069 6E63 6C75 6469 6E67 2077"            /* lts, including w */
	$"6974 686F 7574 206C 696D 6974 6174 696F"            /* ithout limitatio */
	$"6E2C 2062 656E 6368 6D61 726B 2074 6573"            /* n, benchmark tes */
	$"7420 7265 7375 6C74 7320 2863 6F6C 6C65"            /* t results (colle */
	$"6374 6976 656C 7920 2250 6572 666F 726D"            /* ctively "Perform */
	$"616E 6365 2044 6174 6122 292C 2072 656C"            /* ance Data"), rel */
	$"6174 696E 6720 746F 2074 6865 2044 6576"            /* ating to the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"6172 6520 7468 6520 436F 6E66 6964 656E"            /* are the Confiden */
	$"7469 616C 2049 6E66 6F72 6D61 7469 6F6E"            /* tial Information */
	$"206F 6620 414E 5343 412C 2061 6E64 2077"            /*  of ANSCA, and w */
	$"696C 6C20 6265 2074 7265 6174 6564 2069"            /* ill be treated i */
	$"6E20 6163 636F 7264 616E 6365 2077 6974"            /* n accordance wit */
	$"6820 7468 6520 7465 726D 7320 6F66 2073"            /* h the terms of s */
	$"6563 7469 6F6E 2035 206F 7220 756E 6465"            /* ection 5 or unde */
	$"7220 616E 7920 6F62 6C69 6761 7469 6F6E"            /* r any obligation */
	$"206F 6620 636F 6E66 6964 656E 7469 616C"            /*  of confidential */
	$"6974 792C 2077 6869 6368 6576 6572 2063"            /* ity, whichever c */
	$"616D 6520 6669 7273 742E 2041 6363 6F72"            /* ame first. Accor */
	$"6469 6E67 6C79 2C20 596F 7520 7368 616C"            /* dingly, You shal */
	$"6C20 6E6F 7420 7075 626C 6973 6820 6F72"            /* l not publish or */
	$"2064 6973 636C 6F73 6520 746F 2061 6E79"            /*  disclose to any */
	$"2074 6869 7264 2070 6172 7479 2061 6E79"            /*  third party any */
	$"2050 6572 666F 726D 616E 6365 2044 6174"            /*  Performance Dat */
	$"6120 7265 6C61 7469 6E67 2074 6F20 7468"            /* a relating to th */
	$"6520 4465 7665 6C6F 7065 7220 536F 6674"            /* e Developer Soft */
	$"7761 7265 2E0A 0A46 2E20 4E6F 204F 7468"            /* ware.ÂÂF. No Oth */
	$"6572 2050 6572 6D69 7474 6564 2055 7365"            /* er Permitted Use */
	$"732E 2045 7863 6570 7420 6173 206F 7468"            /* s. Except as oth */
	$"6572 7769 7365 2073 6574 2066 6F72 7468"            /* erwise set forth */
	$"2069 6E20 7468 6973 2041 6772 6565 6D65"            /*  in this Agreeme */
	$"6E74 2C20 596F 7520 6167 7265 6520 6E6F"            /* nt, You agree no */
	$"7420 746F 2072 656E 742C 206C 6561 7365"            /* t to rent, lease */
	$"2C20 6C65 6E64 2C20 7570 6C6F 6164 2074"            /* , lend, upload t */
	$"6F20 6F72 2068 6F73 7420 6F6E 2061 6E79"            /* o or host on any */
	$"2077 6562 7369 7465 206F 7220 7365 7276"            /*  website or serv */
	$"6572 2C20 7365 6C6C 2C20 7265 6469 7374"            /* er, sell, redist */
	$"7269 6275 7465 2C20 6F72 2073 7562 6C69"            /* ribute, or subli */
	$"6365 6E73 6520 7468 6520 4465 7665 6C6F"            /* cense the Develo */
	$"7065 7220 536F 6674 7761 7265 2C20 696E"            /* per Software, in */
	$"2077 686F 6C65 206F 7220 696E 2070 6172"            /*  whole or in par */
	$"742C 206F 7220 746F 2065 6E61 626C 6520"            /* t, or to enable  */
	$"6F74 6865 7273 2074 6F20 646F 2073 6F2E"            /* others to do so. */
	$"2059 6F75 206D 6179 206E 6F74 2075 7365"            /*  You may not use */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 6520 666F 7220 616E 7920"            /* oftware for any  */
	$"7075 7270 6F73 6520 6E6F 7420 6578 7072"            /* purpose not expr */
	$"6573 736C 7920 7065 726D 6974 7465 6420"            /* essly permitted  */
	$"6279 2074 6869 7320 4167 7265 656D 656E"            /* by this Agreemen */
	$"742E 2059 6F75 2061 6772 6565 206E 6F74"            /* t. You agree not */
	$"2074 6F20 696E 7374 616C 6C2C 2075 7365"            /*  to install, use */
	$"206F 7220 7275 6E20 7468 6520 4465 7665"            /*  or run the Deve */
	$"6C6F 7065 7220 536F 6674 7761 7265 2069"            /* loper Software i */
	$"6E20 636F 6E6E 6563 7469 6F6E 2077 6974"            /* n connection wit */
	$"6820 6465 7669 6365 7320 6F74 6865 7220"            /* h devices other  */
	$"7468 616E 2041 4E53 4341 2D73 7570 706F"            /* than ANSCA-suppo */
	$"7274 6564 2064 6576 6963 6573 2C20 6F72"            /* rted devices, or */
	$"2074 6F20 656E 6162 6C65 206F 7468 6572"            /*  to enable other */
	$"7320 746F 2064 6F20 736F 2E20 596F 7520"            /* s to do so. You  */
	$"6D61 7920 6E6F 7420 616E 6420 596F 7520"            /* may not and You  */
	$"6167 7265 6520 6E6F 7420 746F 2C20 6F72"            /* agree not to, or */
	$"2074 6F20 656E 6162 6C65 206F 7468 6572"            /*  to enable other */
	$"7320 746F 2C20 636F 7079 2028 6578 6365"            /* s to, copy (exce */
	$"7074 2061 7320 6578 7072 6573 736C 7920"            /* pt as expressly  */
	$"7065 726D 6974 7465 6420 756E 6465 7220"            /* permitted under  */
	$"7468 6973 2041 6772 6565 6D65 6E74 292C"            /* this Agreement), */
	$"2064 6563 6F6D 7069 6C65 2C20 7265 7665"            /*  decompile, reve */
	$"7273 6520 656E 6769 6E65 6572 2C20 6469"            /* rse engineer, di */
	$"7361 7373 656D 626C 652C 2061 7474 656D"            /* sassemble, attem */
	$"7074 2074 6F20 6465 7269 7665 2074 6865"            /* pt to derive the */
	$"2073 6F75 7263 6520 636F 6465 206F 662C"            /*  source code of, */
	$"206D 6F64 6966 792C 2064 6563 7279 7074"            /*  modify, decrypt */
	$"2C20 6F72 2063 7265 6174 6520 6465 7269"            /* , or create deri */
	$"7661 7469 7665 2077 6F72 6B73 206F 6620"            /* vative works of  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 206F 7220 616E 7920 7365"            /* ftware or any se */
	$"7276 6963 6573 2070 726F 7669 6465 6420"            /* rvices provided  */
	$"6279 2074 6865 2044 6576 656C 6F70 6572"            /* by the Developer */
	$"2053 6F66 7477 6172 652C 206F 7220 616E"            /*  Software, or an */
	$"7920 7061 7274 2074 6865 7265 6F66 2028"            /* y part thereof ( */
	$"6578 6365 7074 2061 7320 616E 6420 6F6E"            /* except as and on */
	$"6C79 2074 6F20 7468 6520 6578 7465 6E74"            /* ly to the extent */
	$"2061 6E79 2066 6F72 6567 6F69 6E67 2072"            /*  any foregoing r */
	$"6573 7472 6963 7469 6F6E 2069 7320 7072"            /* estriction is pr */
	$"6F68 6962 6974 6564 2062 7920 6170 706C"            /* ohibited by appl */
	$"6963 6162 6C65 206C 6177 206F 7220 746F"            /* icable law or to */
	$"2074 6865 2065 7874 656E 7420 6173 206D"            /*  the extent as m */
	$"6179 2062 6520 7065 726D 6974 7465 6420"            /* ay be permitted  */
	$"6279 206C 6963 656E 7369 6E67 2074 6572"            /* by licensing ter */
	$"6D73 2067 6F76 6572 6E69 6E67 2075 7365"            /* ms governing use */
	$"206F 6620 4F70 656E 2053 6F75 7263 6564"            /*  of Open Sourced */
	$"2043 6F6D 706F 6E65 6E74 7320 6F72 2053"            /*  Components or S */
	$"616D 706C 6520 436F 6465 2069 6E63 6C75"            /* ample Code inclu */
	$"6465 6420 7769 7468 2074 6865 2053 444B"            /* ded with the SDK */
	$"292E 2059 6F75 2061 6772 6565 206E 6F74"            /* ). You agree not */
	$"2074 6F20 6578 706C 6F69 7420 616E 7920"            /*  to exploit any  */
	$"7365 7276 6963 6573 2070 726F 7669 6465"            /* services provide */
	$"6420 6279 2074 6865 2053 444B 2069 6E20"            /* d by the SDK in  */
	$"616E 7920 756E 6175 7468 6F72 697A 6564"            /* any unauthorized */
	$"2077 6179 2077 6861 7473 6F65 7665 722C"            /*  way whatsoever, */
	$"2069 6E63 6C75 6469 6E67 2062 7574 206E"            /*  including but n */
	$"6F74 206C 696D 6974 6564 2074 6F2C 2062"            /* ot limited to, b */
	$"7920 7472 6573 7061 7373 206F 7220 6275"            /* y trespass or bu */
	$"7264 656E 696E 6720 6E65 7477 6F72 6B20"            /* rdening network  */
	$"6361 7061 6369 7479 2E20 416E 7920 6174"            /* capacity. Any at */
	$"7465 6D70 7420 746F 2064 6F20 736F 2069"            /* tempt to do so i */
	$"7320 6120 7669 6F6C 6174 696F 6E20 6F66"            /* s a violation of */
	$"2074 6865 2072 6967 6874 7320 6F66 2041"            /*  the rights of A */
	$"4E53 4341 2061 6E64 2069 7473 206C 6963"            /* NSCA and its lic */
	$"656E 736F 7273 206F 6620 7468 6520 4465"            /* ensors of the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"206F 7220 7365 7276 6963 6573 2070 726F"            /*  or services pro */
	$"7669 6465 6420 6279 2074 6865 2044 6576"            /* vided by the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 652E"            /* eloper Software. */
	$"2049 6620 596F 7520 6272 6561 6368 2061"            /*  If You breach a */
	$"6E79 206F 6620 7468 6520 666F 7265 676F"            /* ny of the forego */
	$"696E 6720 7265 7374 7269 6374 696F 6E73"            /* ing restrictions */
	$"2C20 596F 7520 6D61 7920 6265 2073 7562"            /* , You may be sub */
	$"6A65 6374 2074 6F20 7072 6F73 6563 7574"            /* ject to prosecut */
	$"696F 6E20 616E 6420 6461 6D61 6765 732E"            /* ion and damages. */
	$"2041 6C6C 206C 6963 656E 7365 7320 6E6F"            /*  All licenses no */
	$"7420 6578 7072 6573 736C 7920 6772 616E"            /* t expressly gran */
	$"7465 6420 696E 2074 6869 7320 4167 7265"            /* ted in this Agre */
	$"656D 656E 7420 6172 6520 7265 7365 7276"            /* ement are reserv */
	$"6564 2061 6E64 206E 6F20 6F74 6865 7220"            /* ed and no other  */
	$"6C69 6365 6E73 6573 2C20 696D 6D75 6E69"            /* licenses, immuni */
	$"7479 206F 7220 7269 6768 7473 2C20 6578"            /* ty or rights, ex */
	$"7072 6573 7320 6F72 2069 6D70 6C69 6564"            /* press or implied */
	$"2061 7265 2067 7261 6E74 6564 2062 7920"            /*  are granted by  */
	$"414E 5343 412C 2062 7920 696D 706C 6963"            /* ANSCA, by implic */
	$"6174 696F 6E2C 2065 7374 6F70 7065 6C2C"            /* ation, estoppel, */
	$"206F 7220 6F74 6865 7277 6973 652E 2054"            /*  or otherwise. T */
	$"6869 7320 4167 7265 656D 656E 7420 646F"            /* his Agreement do */
	$"6573 206E 6F74 2067 7261 6E74 2059 6F75"            /* es not grant You */
	$"2061 6E79 2072 6967 6874 7320 746F 2075"            /*  any rights to u */
	$"7365 2061 6E79 2074 7261 6465 6D61 726B"            /* se any trademark */
	$"732C 206C 6F67 6F73 206F 7220 7365 7276"            /* s, logos or serv */
	$"6963 6520 6D61 726B 7320 6265 6C6F 6E67"            /* ice marks belong */
	$"696E 6720 746F 2041 4E53 4341 2ECA 0A0A"            /* ing to ANSCA.ÊÂÂ */
	$"472E 204F 7468 6572 2052 6573 7472 6963"            /* G. Other Restric */
	$"7469 6F6E 732E CA20 4578 6365 7074 2061"            /* tions.Ê Except a */
	$"7320 616E 6420 6F6E 6C79 2074 6F20 7468"            /* s and only to th */
	$"6520 6578 7465 6E74 2065 7870 7265 7373"            /* e extent express */
	$"6C79 2070 6572 6D69 7474 6564 2062 7920"            /* ly permitted by  */
	$"7468 6973 204C 6963 656E 7365 2C20 6279"            /* this License, by */
	$"2061 7070 6C69 6361 626C 6520 6C69 6365"            /*  applicable lice */
	$"6E73 696E 6720 7465 726D 7320 676F 7665"            /* nsing terms gove */
	$"726E 696E 6720 7573 6520 6F66 2074 6865"            /* rning use of the */
	$"204F 7065 6E20 536F 7572 6365 6420 436F"            /*  Open Sourced Co */
	$"6D70 6F6E 656E 7473 206F 7220 5361 6D70"            /* mponents or Samp */
	$"6C65 2043 6F64 652C 206F 7220 746F 2074"            /* le Code, or to t */
	$"6865 2065 7874 656E 7420 7468 6174 2074"            /* he extent that t */
	$"6865 2066 6F6C 6C6F 7769 6E67 2072 6573"            /* he following res */
	$"7472 6963 7469 6F6E 7320 6172 6520 7072"            /* trictions are pr */
	$"6F68 6962 6974 6564 2062 7920 6170 706C"            /* ohibited by appl */
	$"6963 6162 6C65 206C 6177 2C20 596F 7520"            /* icable law, You  */
	$"6D61 7920 6E6F 7420 636F 7079 2C20 6465"            /* may not copy, de */
	$"636F 6D70 696C 652C 2072 6576 6572 7365"            /* compile, reverse */
	$"2065 6E67 696E 6565 722C 2064 6973 6173"            /*  engineer, disas */
	$"7365 6D62 6C65 2C20 6174 7465 6D70 7420"            /* semble, attempt  */
	$"746F 2064 6572 6976 6520 7468 6520 736F"            /* to derive the so */
	$"7572 6365 2063 6F64 6520 6F66 2074 6865"            /* urce code of the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652C 206D 6F64 6966 792C 2064 6563"            /* are, modify, dec */
	$"7279 7074 2C20 6372 6561 7465 2064 6572"            /* rypt, create der */
	$"6976 6174 6976 6520 776F 726B 7320 6F66"            /* ivative works of */
	$"2C20 696E 636F 7270 6F72 6174 6520 696E"            /* , incorporate in */
	$"746F 206F 7220 636F 6D70 696C 6520 696E"            /* to or compile in */
	$"2063 6F6D 6269 6E61 7469 6F6E 2077 6974"            /*  combination wit */
	$"6820 596F 7572 206F 776E 2070 726F 6772"            /* h Your own progr */
	$"616D 732C 2073 7562 6C69 6365 6E73 6520"            /* ams, sublicense  */
	$"6F72 206F 7468 6572 7769 7365 2072 6564"            /* or otherwise red */
	$"6973 7472 6962 7574 6520 7468 6520 4465"            /* istribute the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2ECA 0A0A 0A33 2E20 596F 7572 204F 626C"            /* .ÊÂÂÂ3. Your Obl */
	$"6967 6174 696F 6E73 0A0A 412E 2047 656E"            /* igationsÂÂA. Gen */
	$"6572 616C 2E20 596F 7520 6365 7274 6966"            /* eral. You certif */
	$"7920 746F 2041 4E53 4341 2061 6E64 2061"            /* y to ANSCA and a */
	$"6772 6565 2074 6861 743A CA0A 2861 2920"            /* gree that:ÊÂ(a)  */
	$"596F 7520 6172 6520 6F66 2074 6865 206C"            /* You are of the l */
	$"6567 616C 2061 6765 206F 6620 6D61 6A6F"            /* egal age of majo */
	$"7269 7479 2069 6E20 7468 6520 6A75 7269"            /* rity in the juri */
	$"7364 6963 7469 6F6E 2069 6E20 7768 6963"            /* sdiction in whic */
	$"6820 596F 7520 7265 7369 6465 2028 6174"            /* h You reside (at */
	$"206C 6561 7374 2031 3820 7965 6172 7320"            /*  least 18 years  */
	$"6F66 2061 6765 2069 6E20 6D61 6E79 2063"            /* of age in many c */
	$"6F75 6E74 7269 6573 2920 616E 6420 6861"            /* ountries) and ha */
	$"7665 2074 6865 2072 6967 6874 2061 6E64"            /* ve the right and */
	$"2061 7574 686F 7269 7479 2074 6F20 656E"            /*  authority to en */
	$"7465 7220 696E 746F 2074 6869 7320 4167"            /* ter into this Ag */
	$"7265 656D 656E 7420 6F6E 2059 6F75 7220"            /* reement on Your  */
	$"6F77 6E20 6265 6861 6C66 2C20 6F72 2069"            /* own behalf, or i */
	$"6620 596F 7520 6172 6520 656E 7465 7269"            /* f You are enteri */
	$"6E67 2069 6E74 6F20 7468 6973 2041 6772"            /* ng into this Agr */
	$"6565 6D65 6E74 206F 6E20 6265 6861 6C66"            /* eement on behalf */
	$"206F 6620 596F 7572 2063 6F6D 7061 6E79"            /*  of Your company */
	$"2C20 6F72 6761 6E69 7A61 7469 6F6E 206F"            /* , organization o */
	$"7220 6564 7563 6174 696F 6E61 6C20 696E"            /* r educational in */
	$"7374 6974 7574 696F 6E2C 2074 6861 7420"            /* stitution, that  */
	$"596F 7520 6861 7665 2074 6865 2072 6967"            /* You have the rig */
	$"6874 2061 6E64 2061 7574 686F 7269 7479"            /* ht and authority */
	$"2074 6F20 6C65 6761 6C6C 7920 6269 6E64"            /*  to legally bind */
	$"2059 6F75 7220 636F 6D70 616E 792C 206F"            /*  Your company, o */
	$"7267 616E 697A 6174 696F 6E20 6F72 2065"            /* rganization or e */
	$"6475 6361 7469 6F6E 616C 2069 6E73 7469"            /* ducational insti */
	$"7475 7469 6F6E 2074 6F20 7468 6520 7465"            /* tution to the te */
	$"726D 7320 616E 6420 6F62 6C69 6761 7469"            /* rms and obligati */
	$"6F6E 7320 6F66 2074 6869 7320 4167 7265"            /* ons of this Agre */
	$"656D 656E 743B CA0A 2862 2920 416C 6C20"            /* ement;ÊÂ(b) All  */
	$"696E 666F 726D 6174 696F 6E20 7072 6F76"            /* information prov */
	$"6964 6564 2062 7920 596F 7520 746F 2041"            /* ided by You to A */
	$"4E53 4341 206F 7220 596F 7572 2065 6E64"            /* NSCA or Your end */
	$"2075 7365 7273 2069 6E20 636F 6E6E 6563"            /*  users in connec */
	$"7469 6F6E 2077 6974 6820 7468 6973 2041"            /* tion with this A */
	$"6772 6565 6D65 6E74 206F 7220 596F 7572"            /* greement or Your */
	$"2041 7070 6C69 6361 7469 6F6E 2C20 696E"            /*  Application, in */
	$"636C 7564 696E 6720 7769 7468 6F75 7420"            /* cluding without  */
	$"6C69 6D69 7461 7469 6F6E 204C 6963 656E"            /* limitation Licen */
	$"7365 6420 4170 706C 6963 6174 696F 6E20"            /* sed Application  */
	$"496E 666F 726D 6174 696F 6E2C 2077 696C"            /* Information, wil */
	$"6C20 6265 2063 7572 7265 6E74 2C20 7472"            /* l be current, tr */
	$"7565 2C20 6163 6375 7261 7465 2061 6E64"            /* ue, accurate and */
	$"2063 6F6D 706C 6574 6520 616E 642C 2077"            /*  complete and, w */
	$"6974 6820 7265 6761 7264 2074 6F20 696E"            /* ith regard to in */
	$"666F 726D 6174 696F 6E20 596F 7520 7072"            /* formation You pr */
	$"6F76 6964 6520 746F 2041 4E53 4341 2C20"            /* ovide to ANSCA,  */
	$"596F 7520 7769 6C6C 2070 726F 6D70 746C"            /* You will promptl */
	$"7920 6E6F 7469 6679 2041 4E53 4341 206F"            /* y notify ANSCA o */
	$"6620 616E 7920 6368 616E 6765 7320 746F"            /* f any changes to */
	$"2073 7563 6820 696E 666F 726D 6174 696F"            /*  such informatio */
	$"6E3B CA0A 2863 2920 596F 7520 7769 6C6C"            /* n;ÊÂ(c) You will */
	$"2063 6F6D 706C 7920 7769 7468 2074 6865"            /*  comply with the */
	$"2074 6572 6D73 206F 6620 616E 6420 6675"            /*  terms of and fu */
	$"6C66 696C 6C20 596F 7572 206F 626C 6967"            /* lfill Your oblig */
	$"6174 696F 6E73 2075 6E64 6572 2074 6869"            /* ations under thi */
	$"7320 4167 7265 656D 656E 7420 616E 6420"            /* s Agreement and  */
	$"596F 7520 6167 7265 6520 746F 206D 6F6E"            /* You agree to mon */
	$"6974 6F72 2061 6E64 2062 6520 7265 7370"            /* itor and be resp */
	$"6F6E 7369 626C 6520 666F 7220 596F 7572"            /* onsible for Your */
	$"2041 7574 686F 7269 7A65 6420 4465 7665"            /*  Authorized Deve */
	$"6C6F 7065 7273 2720 7573 6520 6F66 2074"            /* lopers' use of t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 6520 616E 6420 7468 6569 7220"            /* tware and their  */
	$"636F 6D70 6C69 616E 6365 2077 6974 6820"            /* compliance with  */
	$"7468 6520 7465 726D 7320 6F66 2074 6869"            /* the terms of thi */
	$"7320 4167 7265 656D 656E 743B CA0A 2864"            /* s Agreement;ÊÂ(d */
	$"2920 596F 7520 7769 6C6C 2062 6520 736F"            /* ) You will be so */
	$"6C65 6C79 2072 6573 706F 6E73 6962 6C65"            /* lely responsible */
	$"2066 6F72 2061 6C6C 2063 6F73 7473 2C20"            /*  for all costs,  */
	$"6578 7065 6E73 6573 2C20 6C6F 7373 6573"            /* expenses, losses */
	$"2061 6E64 206C 6961 6269 6C69 7469 6573"            /*  and liabilities */
	$"2069 6E63 7572 7265 642C 2061 6E64 2061"            /*  incurred, and a */
	$"6374 6976 6974 6965 7320 756E 6465 7274"            /* ctivities undert */
	$"616B 656E 2062 7920 596F 7520 616E 6420"            /* aken by You and  */
	$"4175 7468 6F72 697A 6564 2044 6576 656C"            /* Authorized Devel */
	$"6F70 6572 7320 696E 2063 6F6E 6E65 6374"            /* opers in connect */
	$"696F 6E20 7769 7468 2074 6865 2044 6576"            /* ion with the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 653B"            /* eloper Software; */
	$"0A28 6529 2059 6F75 2077 696C 6C20 6E6F"            /* Â(e) You will no */
	$"7420 6163 7420 696E 2061 6E79 206D 616E"            /* t act in any man */
	$"6E65 7220 7768 6963 6820 636F 6EDF 6963"            /* ner which conßic */
	$"7473 206F 7220 696E 7465 7266 6572 6573"            /* ts or interferes */
	$"2077 6974 6820 616E 7920 6578 6973 7469"            /*  with any existi */
	$"6E67 2063 6F6D 6D69 746D 656E 7420 6F72"            /* ng commitment or */
	$"206F 626C 6967 6174 696F 6E2E 2059 6F75"            /*  obligation. You */
	$"206D 6179 2068 6176 6520 616E 6420 6E6F"            /*  may have and no */
	$"2061 6772 6565 6D65 6E74 2070 7265 7669"            /*  agreement previ */
	$"6F75 736C 7920 656E 7465 7265 6420 696E"            /* ously entered in */
	$"746F 2062 7920 596F 7520 7769 6C6C 2069"            /* to by You will i */
	$"6E74 6572 6665 7265 2077 6974 6820 596F"            /* nterfere with Yo */
	$"7572 2070 6572 666F 726D 616E 6365 206F"            /* ur performance o */
	$"6620 596F 7572 206F 626C 6967 6174 696F"            /* f Your obligatio */
	$"6E73 2075 6E64 6572 2074 6869 7320 4167"            /* ns under this Ag */
	$"7265 656D 656E 742E 0A0A 422E 2052 6571"            /* reement.ÂÂB. Req */
	$"7569 7265 6D65 6E74 7320 666F 7220 4170"            /* uirements for Ap */
	$"706C 6963 6174 696F 6E73 2ECA 2041 6E79"            /* plications.Ê Any */
	$"2041 7070 6C69 6361 7469 6F6E 2064 6576"            /*  Application dev */
	$"656C 6F70 6564 2075 7369 6E67 2074 6865"            /* eloped using the */
	$"2053 444B 206D 7573 7420 6D65 6574 2061"            /*  SDK must meet a */
	$"6C6C 206F 6620 7468 6520 666F 6C6C 6F77"            /* ll of the follow */
	$"696E 6720 6372 6974 6572 6961 2061 6E64"            /* ing criteria and */
	$"2072 6571 7569 7265 6D65 6E74 732C 2061"            /*  requirements, a */
	$"7320 7468 6579 206D 6179 2062 6520 6D6F"            /* s they may be mo */
	$"6469 6669 6564 2062 7920 414E 5343 4120"            /* dified by ANSCA  */
	$"6672 6F6D 2074 696D 6520 746F 2074 696D"            /* from time to tim */
	$"653A 0A0A 2861 2920 4150 4973 2061 6E64"            /* e:ÂÂ(a) APIs and */
	$"2046 756E 6374 696F 6E61 6C69 7479 3A0A"            /*  Functionality:Â */
	$"2A20 4170 706C 6963 6174 696F 6E73 206D"            /* * Applications m */
	$"6179 206F 6E6C 7920 7573 6520 446F 6375"            /* ay only use Docu */
	$"6D65 6E74 6564 2041 5049 7320 696E 2074"            /* mented APIs in t */
	$"6865 206D 616E 6E65 7220 7072 6573 6372"            /* he manner prescr */
	$"6962 6564 2062 7920 414E 5343 4120 616E"            /* ibed by ANSCA an */
	$"6420 6D75 7374 206E 6F74 2075 7365 206F"            /* d must not use o */
	$"7220 6361 6C6C 2061 6E79 2070 7269 7661"            /* r call any priva */
	$"7465 2041 5049 732E CA0A 2A20 416E 2041"            /* te APIs.ÊÂ* An A */
	$"7070 6C69 6361 7469 6F6E 206D 6179 206E"            /* pplication may n */
	$"6F74 2069 7473 656C 6620 696E 7374 616C"            /* ot itself instal */
	$"6C20 6F72 206C 6175 6E63 6820 6F74 6865"            /* l or launch othe */
	$"7220 6578 6563 7574 6162 6C65 2063 6F64"            /* r executable cod */
	$"6520 6279 2061 6E79 206D 6561 6E73 2ECA"            /* e by any means.Ê */
	$"0A2A 2041 6E20 4170 706C 6963 6174 696F"            /* Â* An Applicatio */
	$"6E20 6D61 7920 7772 6974 6520 6461 7461"            /* n may write data */
	$"206F 6E20 6120 6465 7669 6365 206F 6E6C"            /*  on a device onl */
	$"7920 746F 2074 6865 2041 7070 6C69 6361"            /* y to the Applica */
	$"7469 6F6E 2773 2064 6573 6967 6E61 7465"            /* tion's designate */
	$"6420 636F 6E74 6169 6E65 7220 6172 6561"            /* d container area */
	$"2C20 6578 6365 7074 2061 7320 6F74 6865"            /* , except as othe */
	$"7277 6973 6520 7370 6563 6966 6965 6420"            /* rwise specified  */
	$"6279 2041 4E53 4341 2ECA 0A0A 2862 2920"            /* by ANSCA.ÊÂÂ(b)  */
	$"5573 6572 2049 6E74 6572 6661 6365 2061"            /* User Interface a */
	$"6E64 2044 6174 613A 2041 6E79 2066 6F72"            /* nd Data: Any for */
	$"6D20 6F66 2075 7365 7220 6F72 2064 6576"            /* m of user or dev */
	$"6963 6520 6461 7461 2063 6F6C 6C65 6374"            /* ice data collect */
	$"696F 6E2C 206F 7220 696D 6167 652C 2070"            /* ion, or image, p */
	$"6963 7475 7265 206F 7220 766F 6963 6520"            /* icture or voice  */
	$"6361 7074 7572 6520 6F72 2072 6563 6F72"            /* capture or recor */
	$"6469 6E67 2070 6572 666F 726D 6564 2062"            /* ding performed b */
	$"7920 7468 6520 4170 706C 6963 6174 696F"            /* y the Applicatio */
	$"6E20 2863 6F6C 6C65 6374 6976 656C 7920"            /* n (collectively  */
	$"2252 6563 6F72 6469 6E67 7322 292C 2061"            /* "Recordings"), a */
	$"6E64 2061 6E79 2066 6F72 6D20 6F66 2075"            /* nd any form of u */
	$"7365 7220 6461 7461 2C20 636F 6E74 656E"            /* ser data, conten */
	$"7420 6F72 2069 6E66 6F72 6D61 7469 6F6E"            /* t or information */
	$"2070 726F 6365 7373 696E 672C 206D 6169"            /*  processing, mai */
	$"6E74 656E 616E 6365 2C20 7570 6C6F 6164"            /* ntenance, upload */
	$"696E 672C 2073 796E 6369 6E67 2C20 6F72"            /* ing, syncing, or */
	$"2074 7261 6E73 6D69 7373 696F 6E20 7065"            /*  transmission pe */
	$"7266 6F72 6D65 6420 6279 2074 6865 2041"            /* rformed by the A */
	$"7070 6C69 6361 7469 6F6E 2028 636F 6C6C"            /* pplication (coll */
	$"6563 7469 7665 6C79 2022 5472 616E 736D"            /* ectively "Transm */
	$"6973 7369 6F6E 7322 2920 6D75 7374 2063"            /* issions") must c */
	$"6F6D 706C 7920 7769 7468 2061 6C6C 2061"            /* omply with all a */
	$"7070 6C69 6361 626C 6520 7072 6976 6163"            /* pplicable privac */
	$"7920 6C61 7773 2061 6E64 2072 6567 756C"            /* y laws and regul */
	$"6174 696F 6E73 2061 7320 7765 6C6C 2061"            /* ations as well a */
	$"7320 616E 7920 416E 7363 6120 7072 6F67"            /* s any Ansca prog */
	$"7261 6D20 7265 7175 6972 656D 656E 7473"            /* ram requirements */
	$"2072 656C 6174 6564 2074 6F20 7375 6368"            /*  related to such */
	$"2061 7370 6563 7473 2C20 696E 636C 7564"            /*  aspects, includ */
	$"696E 6720 6275 7420 6E6F 7420 6C69 6D69"            /* ing but not limi */
	$"7465 6420 746F 2061 6E79 206E 6F74 6963"            /* ted to any notic */
	$"6520 6F72 2063 6F6E 7365 6E74 2072 6571"            /* e or consent req */
	$"7569 7265 6D65 6E74 732E 2049 6E20 7061"            /* uirements. In pa */
	$"7274 6963 756C 6172 2C20 6120 7265 6173"            /* rticular, a reas */
	$"6F6E 6162 6C79 2063 6F6E 7370 6963 756F"            /* onably conspicuo */
	$"7573 2076 6973 7561 6C20 696E 6469 6361"            /* us visual indica */
	$"746F 7220 6D75 7374 2062 65CA 0A64 6973"            /* tor must beÊÂdis */
	$"706C 6179 6564 2074 6F20 7468 6520 7573"            /* played to the us */
	$"6572 2061 7320 7061 7274 206F 6620 7468"            /* er as part of th */
	$"6520 4170 706C 6963 6174 696F 6E20 746F"            /* e Application to */
	$"2069 6E64 6963 6174 6520 7468 6174 2061"            /*  indicate that a */
	$"2052 6563 6F72 6469 6E67 2069 7320 7461"            /*  Recording is ta */
	$"6B69 6E67 2070 6C61 6365 2ECA 0A0A 2863"            /* king place.ÊÂÂ(c */
	$"2920 4C6F 6361 7469 6F6E 2053 6572 7669"            /* ) Location Servi */
	$"6365 7320 616E 6420 5573 6572 2050 7269"            /* ces and User Pri */
	$"7661 6379 2E20 466F 7220 4170 706C 6963"            /* vacy. For Applic */
	$"6174 696F 6E73 2074 6861 7420 7573 6520"            /* ations that use  */
	$"6C6F 6361 7469 6F6E 2D62 6173 6564 2041"            /* location-based A */
	$"5049 732C 206F 6666 6572 206C 6F63 6174"            /* PIs, offer locat */
	$"696F 6E2D 6261 7365 6420 7365 7276 6963"            /* ion-based servic */
	$"6573 206F 7220 6675 6E63 7469 6F6E 616C"            /* es or functional */
	$"6974 792C 206F 7220 7468 6174 2063 6F6C"            /* ity, or that col */
	$"6C65 6374 2C20 7472 616E 736D 6974 2C20"            /* lect, transmit,  */
	$"6D61 696E 7461 696E 2C20 7072 6F63 6573"            /* maintain, proces */
	$"732C 2073 6861 7265 2C20 6469 7363 6C6F"            /* s, share, disclo */
	$"7365 206F 7220 6F74 6865 7277 6973 6520"            /* se or otherwise  */
	$"7573 6520 6120 7573 6572 2773 2070 6572"            /* use a user's per */
	$"736F 6E61 6C20 696E 666F 726D 6174 696F"            /* sonal informatio */
	$"6E3A CA0A 2A20 596F 7520 616E 6420 7468"            /* n:ÊÂ* You and th */
	$"6520 4170 706C 6963 6174 696F 6E20 6D75"            /* e Application mu */
	$"7374 2063 6F6D 706C 7920 7769 7468 2061"            /* st comply with a */
	$"6C6C 2061 7070 6C69 6361 626C 6520 7072"            /* ll applicable pr */
	$"6976 6163 7920 616E 6420 6461 7461 2063"            /* ivacy and data c */
	$"6F6C 6C65 6374 696F 6E20 6C61 7773 2061"            /* ollection laws a */
	$"6E64 2072 6567 756C 6174 696F 6E73 2077"            /* nd regulations w */
	$"6974 6820 7265 7370 6563 7420 746F 2061"            /* ith respect to a */
	$"6E79 2063 6F6C 6C65 6374 696F 6E2C 2074"            /* ny collection, t */
	$"7261 6E73 6D69 7373 696F 6E2C CA0A 6D61"            /* ransmission,ÊÂma */
	$"696E 7465 6E61 6E63 652C 2070 726F 6365"            /* intenance, proce */
	$"7373 696E 672C 2075 7365 2C20 6574 632E"            /* ssing, use, etc. */
	$"206F 6620 7468 6520 7573 6572 2773 206C"            /*  of the user's l */
	$"6F63 6174 696F 6E20 6461 7461 206F 7220"            /* ocation data or  */
	$"7065 7273 6F6E 616C 2069 6E66 6F72 6D61"            /* personal informa */
	$"7469 6F6E 2062 7920 7468 6520 4170 706C"            /* tion by the Appl */
	$"6963 6174 696F 6E2E CA0A 2A20 4170 706C"            /* ication.ÊÂ* Appl */
	$"6963 6174 696F 6E73 206D 6179 206E 6F74"            /* ications may not */
	$"2062 6520 6465 7369 676E 6564 206F 7220"            /*  be designed or  */
	$"6D61 726B 6574 6564 2066 6F72 2074 6865"            /* marketed for the */
	$"2070 7572 706F 7365 206F 6620 6861 7261"            /*  purpose of hara */
	$"7373 696E 672C 2061 6275 7369 6E67 2C20"            /* ssing, abusing,  */
	$"7374 616C 6B69 6E67 2C20 7468 7265 6174"            /* stalking, threat */
	$"656E 696E 6720 6F72 206F 7468 6572 7769"            /* ening or otherwi */
	$"7365 2076 696F 6C61 7469 6E67 2074 6865"            /* se violating the */
	$"206C 6567 616C 2072 6967 6874 7320 2873"            /*  legal rights (s */
	$"7563 68CA 0A61 7320 7468 6520 7269 6768"            /* uchÊÂas the righ */
	$"7473 206F 6620 7072 6976 6163 7920 616E"            /* ts of privacy an */
	$"6420 7075 626C 6963 6974 7929 206F 6620"            /* d publicity) of  */
	$"6F74 6865 7273 2ECA 0A2A 2046 6F72 2041"            /* others.ÊÂ* For A */
	$"7070 6C69 6361 7469 6F6E 7320 7468 6174"            /* pplications that */
	$"2075 7365 206C 6F63 6174 696F 6E2D 6261"            /*  use location-ba */
	$"7365 6420 4150 4973 2C20 7375 6368 2041"            /* sed APIs, such A */
	$"7070 6C69 6361 7469 6F6E 7320 6D61 7920"            /* pplications may  */
	$"6E6F 7420 6265 2064 6573 6967 6E65 6420"            /* not be designed  */
	$"6F72 206D 6172 6B65 7465 6420 666F 7220"            /* or marketed for  */
	$"7265 616C 2074 696D 6520 726F 7574 6520"            /* real time route  */
	$"6775 6964 616E 6365 3B20 6175 746F 6D61"            /* guidance; automa */
	$"7469 6320 6F72 CA0A 6175 746F 6E6F 6D6F"            /* tic orÊÂautonomo */
	$"7573 2063 6F6E 7472 6F6C 206F 6620 7665"            /* us control of ve */
	$"6869 636C 6573 2C20 6169 7263 7261 6674"            /* hicles, aircraft */
	$"2C20 6F72 206F 7468 6572 206D 6563 6861"            /* , or other mecha */
	$"6E69 6361 6C20 6465 7669 6365 733B 2064"            /* nical devices; d */
	$"6973 7061 7463 6820 6F72 2066 6C65 6574"            /* ispatch or fleet */
	$"206D 616E 6167 656D 656E 743B 206F 7220"            /*  management; or  */
	$"656D 6572 6765 6E63 7920 6F72 206C 6966"            /* emergency or lif */
	$"652D 7361 7669 6E67 2070 7572 706F 7365"            /* e-saving purpose */
	$"732E CA0A 2A20 4170 706C 6963 6174 696F"            /* s.ÊÂ* Applicatio */
	$"6E73 206D 6179 206E 6F74 2075 7365 2061"            /* ns may not use a */
	$"6E79 2072 6F62 6F74 2C20 7370 6964 6572"            /* ny robot, spider */
	$"2C20 7369 7465 2073 6561 7263 6820 6F72"            /* , site search or */
	$"206F 7468 6572 2072 6574 7269 6576 616C"            /*  other retrieval */
	$"2061 7070 6C69 6361 7469 6F6E 206F 7220"            /*  application or  */
	$"6465 7669 6365 2074 6F20 7363 7261 7065"            /* device to scrape */
	$"2C20 7265 7472 6965 7665 206F 7220 696E"            /* , retrieve or in */
	$"6465 7820 7365 7276 6963 6573 2070 726F"            /* dex services pro */
	$"7669 6465 6420 6279 2041 6E73 6361 206F"            /* vided by Ansca o */
	$"72CA 0A69 7473 206C 6963 656E 736F 7273"            /* rÊÂits licensors */
	$"2C20 6F72 2074 6F20 636F 6C6C 6563 7420"            /* , or to collect  */
	$"696E 666F 726D 6174 696F 6E20 6162 6F75"            /* information abou */
	$"7420 7573 6572 7320 666F 7220 616E 7920"            /* t users for any  */
	$"756E 6175 7468 6F72 697A 6564 2070 7572"            /* unauthorized pur */
	$"706F 7365 2ECA 0A2A 2041 7070 6C69 6361"            /* pose.ÊÂ* Applica */
	$"7469 6F6E 7320 7468 6174 206F 6666 6572"            /* tions that offer */
	$"206C 6F63 6174 696F 6E2D 6261 7365 6420"            /*  location-based  */
	$"7365 7276 6963 6573 206F 7220 6675 6E63"            /* services or func */
	$"7469 6F6E 616C 6974 7920 6D75 7374 206E"            /* tionality must n */
	$"6F74 6966 7920 616E 6420 6F62 7461 696E"            /* otify and obtain */
	$"2063 6F6E 7365 6E74 2066 726F 6D20 616E"            /*  consent from an */
	$"2069 6E64 6976 6964 7561 6C20 6265 666F"            /*  individual befo */
	$"7265 2068 6973 206F 7220 6865 7220 6C6F"            /* re his or her lo */
	$"6361 7469 6F6E 2064 6174 6120 6973 CA0A"            /* cation data isÊÂ */
	$"6265 696E 6720 636F 6C6C 6563 7465 642C"            /* being collected, */
	$"2074 7261 6E73 6D69 7474 6564 206F 7220"            /*  transmitted or  */
	$"6F74 6865 7277 6973 6520 7573 6564 2062"            /* otherwise used b */
	$"7920 7468 6520 4170 706C 6963 6174 696F"            /* y the Applicatio */
	$"6E2E CA0A 0A28 6429 2043 6F6E 7465 6E74"            /* n.ÊÂÂ(d) Content */
	$"2061 6E64 204D 6174 6572 6961 6C73 3ACA"            /*  and Materials:Ê */
	$"0A2A 2041 6E79 206D 6173 7465 7220 7265"            /* Â* Any master re */
	$"636F 7264 696E 6773 2061 6E64 206D 7573"            /* cordings and mus */
	$"6963 616C 2063 6F6D 706F 7369 7469 6F6E"            /* ical composition */
	$"7320 656D 626F 6469 6564 2069 6E20 596F"            /* s embodied in Yo */
	$"7572 2041 7070 6C69 6361 7469 6F6E 206D"            /* ur Application m */
	$"7573 7420 6265 2077 686F 6C6C 792D 6F77"            /* ust be wholly-ow */
	$"6E65 6420 6279 2059 6F75 206F 7220 6C69"            /* ned by You or li */
	$"6365 6E73 6564 2074 6F20 596F 7520 6F6E"            /* censed to You on */
	$"2061 2066 756C 6C79 2070 6169 64CA 0A75"            /*  a fully paidÊÂu */
	$"7020 6261 7369 7320 616E 6420 696E 2061"            /* p basis and in a */
	$"206D 616E 6E65 7220 7468 6174 2077 696C"            /*  manner that wil */
	$"6C20 6E6F 7420 7265 7175 6972 6520 7468"            /* l not require th */
	$"6520 7061 796D 656E 7420 6F66 2061 6E79"            /* e payment of any */
	$"2066 6565 732C 2072 6F79 616C 7469 6573"            /*  fees, royalties */
	$"2061 6E64 2F6F 7220 7375 6D73 2062 7920"            /*  and/or sums by  */
	$"416E 7363 6120 746F 2059 6F75 206F 7220"            /* Ansca to You or  */
	$"616E 7920 7468 6972 6420 7061 7274 792E"            /* any third party. */
	$"2049 6E20 6164 6469 7469 6F6E 2C20 6966"            /*  In addition, if */
	$"2059 6F75 72CA 0A41 7070 6C69 6361 7469"            /*  YourÊÂApplicati */
	$"6F6E 2077 696C 6C20 6265 2064 6973 7472"            /* on will be distr */
	$"6962 7574 6564 206F 7574 7369 6465 206F"            /* ibuted outside o */
	$"6620 7468 6520 556E 6974 6564 2053 7461"            /* f the United Sta */
	$"7465 732C 2061 6E79 206D 6173 7465 7220"            /* tes, any master  */
	$"7265 636F 7264 696E 6773 2061 6E64 206D"            /* recordings and m */
	$"7573 6963 616C 2063 6F6D 706F 7369 7469"            /* usical compositi */
	$"6F6E 7320 656D 626F 6469 6564 2069 6E20"            /* ons embodied in  */
	$"596F 7572 2041 7070 6C69 6361 7469 6F6E"            /* Your Application */
	$"2028 6129 206D 7573 7420 6E6F 74CA 0A66"            /*  (a) must notÊÂf */
	$"616C 6C20 7769 7468 696E 2074 6865 2072"            /* all within the r */
	$"6570 6572 746F 6972 6520 6F66 2061 6E79"            /* epertoire of any */
	$"206D 6563 6861 6E69 6361 6C20 6F72 2070"            /*  mechanical or p */
	$"6572 666F 726D 696E 672F 636F 6D6D 756E"            /* erforming/commun */
	$"6963 6174 696F 6E20 7269 6768 7473 2063"            /* ication rights c */
	$"6F6C 6C65 6374 696E 6720 6F72 206C 6963"            /* ollecting or lic */
	$"656E 7369 6E67 206F 7267 616E 697A 6174"            /* ensing organizat */
	$"696F 6E20 6E6F 7720 6F72 2069 6E20 7468"            /* ion now or in th */
	$"6520 6675 7475 7265 2061 6E64 2028 6229"            /* e future and (b) */
	$"2069 66CA 0A6C 6963 656E 7365 642C 206D"            /*  ifÊÂlicensed, m */
	$"7573 7420 6265 2065 7863 6C75 7369 7665"            /* ust be exclusive */
	$"6C79 206C 6963 656E 7365 6420 746F 2059"            /* ly licensed to Y */
	$"6F75 2066 6F72 2059 6F75 7220 4170 706C"            /* ou for Your Appl */
	$"6963 6174 696F 6E20 6279 2065 6163 6820"            /* ication by each  */
	$"6170 706C 6963 6162 6C65 2063 6F70 7972"            /* applicable copyr */
	$"6967 6874 206F 776E 6572 2ECA 0A2A 2049"            /* ight owner.ÊÂ* I */
	$"6620 596F 7572 2041 7070 6C69 6361 7469"            /* f Your Applicati */
	$"6F6E 2069 6E63 6C75 6465 7320 6F72 2077"            /* on includes or w */
	$"696C 6C20 696E 636C 7564 6520 616E 7920"            /* ill include any  */
	$"6F74 6865 7220 636F 6E74 656E 742C 2059"            /* other content, Y */
	$"6F75 206D 7573 7420 6569 7468 6572 206F"            /* ou must either o */
	$"776E 2061 6C6C 2073 7563 6820 636F 6E74"            /* wn all such cont */
	$"656E 7420 6F72 2068 6176 6520 7065 726D"            /* ent or have perm */
	$"6973 7369 6F6E 2066 726F 6D20 7468 6520"            /* ission from the  */
	$"636F 6E74 656E 7420 6F77 6E65 7220 746F"            /* content owner to */
	$"CA0A 7573 6520 6974 2069 6E20 596F 7572"            /* ÊÂuse it in Your */
	$"2041 7070 6C69 6361 7469 6F6E 2ECA 0A2A"            /*  Application.ÊÂ* */
	$"2041 7070 6C69 6361 7469 6F6E 7320 6D75"            /*  Applications mu */
	$"7374 206E 6F74 2063 6F6E 7461 696E 2061"            /* st not contain a */
	$"6E79 206D 616C 7761 7265 2C20 6D61 6C69"            /* ny malware, mali */
	$"6369 6F75 7320 6F72 2068 6172 6D66 756C"            /* cious or harmful */
	$"2063 6F64 652C 2070 726F 6772 616D 2C20"            /*  code, program,  */
	$"6F72 206F 7468 6572 2069 6E74 6572 6E61"            /* or other interna */
	$"6C20 636F 6D70 6F6E 656E 7420 2865 2E67"            /* l component (e.g */
	$"2E20 636F 6D70 7574 6572 2076 6972 7573"            /* . computer virus */
	$"6573 2C20 7472 6F6A 616E CA0A 686F 7273"            /* es, trojanÊÂhors */
	$"6573 2C20 2262 6163 6B64 6F6F 7273 2229"            /* es, "backdoors") */
	$"2077 6869 6368 2063 6F75 6C64 2064 616D"            /*  which could dam */
	$"6167 652C 2064 6573 7472 6F79 2C20 6F72"            /* age, destroy, or */
	$"2061 6476 6572 7365 6C79 2061 6666 6563"            /*  adversely affec */
	$"7420 6F74 6865 7220 736F 6674 7761 7265"            /* t other software */
	$"2C20 6669 726D 7761 7265 2C20 6861 7264"            /* , firmware, hard */
	$"7761 7265 2C20 6461 7461 2C20 7379 7374"            /* ware, data, syst */
	$"656D 732C 2073 6572 7669 6365 732C 206F"            /* ems, services, o */
	$"7220 6E65 7477 6F72 6B73 2E0A 0A0A 342E"            /* r networks.ÂÂÂ4. */
	$"2054 7261 6E73 6665 722E 2045 7863 6570"            /*  Transfer. Excep */
	$"7420 6173 206F 7468 6572 7769 7365 2073"            /* t as otherwise s */
	$"6574 2066 6F72 7468 2069 6E20 7468 6973"            /* et forth in this */
	$"2041 6772 6565 6D65 6E74 2C20 596F 7520"            /*  Agreement, You  */
	$"6D61 7920 6E6F 7420 7265 6E74 2C20 6C65"            /* may not rent, le */
	$"6173 652C 206C 656E 642C 2072 6564 6973"            /* ase, lend, redis */
	$"7472 6962 7574 6520 6F72 2073 7562 6C69"            /* tribute or subli */
	$"6365 6E73 6520 7468 6520 4465 7665 6C6F"            /* cense the Develo */
	$"7065 7220 536F 6674 7761 7265 2ECA 2059"            /* per Software.Ê Y */
	$"6F75 206D 6179 2C20 686F 7765 7665 722C"            /* ou may, however, */
	$"206D 616B 6520 6120 6F6E 652D 7469 6D65"            /*  make a one-time */
	$"2070 6572 6D61 6E65 6E74 2074 7261 6E73"            /*  permanent trans */
	$"6665 7220 6F66 2061 6C6C 206F 6620 596F"            /* fer of all of Yo */
	$"7572 206C 6963 656E 7365 2072 6967 6874"            /* ur license right */
	$"7320 746F 2074 6865 2044 6576 656C 6F70"            /* s to the Develop */
	$"6572 2053 6F66 7477 6172 6520 2869 6E20"            /* er Software (in  */
	$"6974 7320 6F72 6967 696E 616C 2066 6F72"            /* its original for */
	$"6D20 6173 2070 726F 7669 6465 6420 6279"            /* m as provided by */
	$"2041 4E53 4341 2920 746F 2061 6E6F 7468"            /*  ANSCA) to anoth */
	$"6572 2070 6172 7479 2C20 7072 6F76 6964"            /* er party, provid */
	$"6564 2074 6861 743A 2028 6129 2074 6865"            /* ed that: (a) the */
	$"2074 7261 6E73 6665 7220 6D75 7374 2069"            /*  transfer must i */
	$"6E63 6C75 6465 2061 6C6C 206F 6620 7468"            /* nclude all of th */
	$"6520 4465 7665 6C6F 7065 7220 536F 6674"            /* e Developer Soft */
	$"7761 7265 2C20 696E 636C 7564 696E 6720"            /* ware, including  */
	$"616C 6C20 6974 7320 636F 6D70 6F6E 656E"            /* all its componen */
	$"7420 7061 7274 732C 206F 7269 6769 6E61"            /* t parts, origina */
	$"6C20 6D65 6469 612C 2070 7269 6E74 6564"            /* l media, printed */
	$"206D 6174 6572 6961 6C73 2061 6E64 2074"            /*  materials and t */
	$"6869 7320 4C69 6365 6E73 653B 2028 6229"            /* his License; (b) */
	$"2059 6F75 2064 6F20 6E6F 7420 7265 7461"            /*  You do not reta */
	$"696E 2061 6E79 2063 6F70 6965 7320 6F66"            /* in any copies of */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 652C 2066 756C 6C20 6F72"            /* oftware, full or */
	$"2070 6172 7469 616C 2C20 696E 636C 7564"            /*  partial, includ */
	$"696E 6720 636F 7069 6573 2073 746F 7265"            /* ing copies store */
	$"6420 6F6E 2061 2063 6F6D 7075 7465 7220"            /* d on a computer  */
	$"6F72 206F 7468 6572 2073 746F 7261 6765"            /* or other storage */
	$"2064 6576 6963 653B 2061 6E64 2028 6329"            /*  device; and (c) */
	$"2074 6865 2070 6172 7479 2072 6563 6569"            /*  the party recei */
	$"7669 6E67 2074 6865 2044 6576 656C 6F70"            /* ving the Develop */
	$"6572 2053 6F66 7477 6172 6520 7265 6164"            /* er Software read */
	$"7320 616E 6420 6167 7265 6573 2074 6F20"            /* s and agrees to  */
	$"6163 6365 7074 2074 6865 2074 6572 6D73"            /* accept the terms */
	$"2061 6E64 2063 6F6E 6469 7469 6F6E 7320"            /*  and conditions  */
	$"6F66 2074 6869 7320 4C69 6365 6E73 652E"            /* of this License. */
	$"2059 6F75 206D 6179 206E 6F74 2072 656E"            /*  You may not ren */
	$"742C 206C 6561 7365 2C20 6C65 6E64 2C20"            /* t, lease, lend,  */
	$"7265 6469 7374 7269 6275 7465 2C20 7375"            /* redistribute, su */
	$"626C 6963 656E 7365 206F 7220 7472 616E"            /* blicense or tran */
	$"7366 6572 2061 6E79 2044 6576 656C 6F70"            /* sfer any Develop */
	$"6572 2053 6F66 7477 6172 6520 7468 6174"            /* er Software that */
	$"2068 6173 2062 6565 6E20 6D6F 6469 6669"            /*  has been modifi */
	$"6564 206F 7220 7265 706C 6163 6564 2075"            /* ed or replaced u */
	$"6E64 6572 2053 6563 7469 6F6E 2032 4220"            /* nder Section 2B  */
	$"6162 6F76 652E 2041 6C6C 2063 6F6D 706F"            /* above. All compo */
	$"6E65 6E74 7320 6F66 2074 6865 2044 6576"            /* nents of the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"6172 6520 7072 6F76 6964 6564 2061 7320"            /* are provided as  */
	$"7061 7274 206F 6620 6120 6275 6E64 6C65"            /* part of a bundle */
	$"2061 6E64 206D 6179 206E 6F74 2062 6520"            /*  and may not be  */
	$"7365 7061 7261 7465 6420 6672 6F6D 2074"            /* separated from t */
	$"6865 2062 756E 646C 6520 616E 6420 6469"            /* he bundle and di */
	$"7374 7269 6275 7465 6420 6173 2073 7461"            /* stributed as sta */
	$"6E64 616C 6F6E 6520 6170 706C 6963 6174"            /* ndalone applicat */
	$"696F 6E73 2E20 CA0A 0A4E 4652 2028 4E6F"            /* ions. ÊÂÂNFR (No */
	$"7420 666F 7220 5265 7361 6C65 2920 616E"            /* t for Resale) an */
	$"6420 4576 616C 7561 7469 6F6E 2043 6F70"            /* d Evaluation Cop */
	$"6965 733A 204E 6F74 7769 7468 7374 616E"            /* ies: Notwithstan */
	$"6469 6E67 206F 7468 6572 2073 6563 7469"            /* ding other secti */
	$"6F6E 7320 6F66 2074 6869 7320 4C69 6365"            /* ons of this Lice */
	$"6E73 652C 2044 6576 656C 6F70 6572 2053"            /* nse, Developer S */
	$"6F66 7477 6172 6520 6C61 6265 6C65 6420"            /* oftware labeled  */
	$"6F72 206F 7468 6572 7769 7365 2070 726F"            /* or otherwise pro */
	$"7669 6465 6420 746F 2059 6F75 206F 6E20"            /* vided to You on  */
	$"6120 7072 6F6D 6F74 696F 6E61 6C20 6261"            /* a promotional ba */
	$"7369 7320 6D61 7920 6F6E 6C79 2062 6520"            /* sis may only be  */
	$"7573 6564 2066 6F72 2064 656D 6F6E 7374"            /* used for demonst */
	$"7261 7469 6F6E 2C20 7465 7374 696E 6720"            /* ration, testing  */
	$"616E 6420 6576 616C 7561 7469 6F6E 2070"            /* and evaluation p */
	$"7572 706F 7365 7320 616E 6420 6D61 7920"            /* urposes and may  */
	$"6E6F 7420 6265 2072 6573 6F6C 6420 6F72"            /* not be resold or */
	$"2074 7261 6E73 6665 7272 6564 2ECA 0A0A"            /*  transferred.ÊÂÂ */
	$"0A35 2E20 5465 726D 2061 6E64 2054 6572"            /* Â5. Term and Ter */
	$"6D69 6E61 7469 6F6E 2ECA 0A0A 412E 2054"            /* mination.ÊÂÂA. T */
	$"6572 6D2E 2059 6F75 7220 7269 6768 7473"            /* erm. Your rights */
	$"2077 6974 6820 7265 7370 6563 7420 746F"            /*  with respect to */
	$"2074 6865 2044 6576 656C 6F70 6572 2053"            /*  the Developer S */
	$"6F66 7477 6172 6520 646F 206E 6F74 2065"            /* oftware do not e */
	$"7874 656E 6420 746F 206F 6E6C 696E 6520"            /* xtend to online  */
	$"7365 7276 6963 6573 2070 726F 7669 6465"            /* services provide */
	$"6420 6279 2041 4E53 4341 2E20 4569 7468"            /* d by ANSCA. Eith */
	$"6572 2070 6172 7479 206D 6179 2074 6572"            /* er party may ter */
	$"6D69 6E61 7465 2074 6869 7320 4C69 6365"            /* minate this Lice */
	$"6E73 6520 6174 2061 6E79 2074 696D 6520"            /* nse at any time  */
	$"666F 7220 616E 7920 7265 6173 6F6E 206F"            /* for any reason o */
	$"7220 6E6F 2072 6561 736F 6E20 6279 2070"            /* r no reason by p */
	$"726F 7669 6469 6E67 2074 6865 206F 7468"            /* roviding the oth */
	$"6572 2070 6172 7479 2061 6476 616E 6365"            /* er party advance */
	$"2077 7269 7474 656E 206E 6F74 6963 6520"            /*  written notice  */
	$"7468 6572 656F 662E 2055 706F 6E20 616E"            /* thereof. Upon an */
	$"7920 6578 7069 7261 7469 6F6E 206F 7220"            /* y expiration or  */
	$"7465 726D 696E 6174 696F 6E20 6F66 2074"            /* termination of t */
	$"6869 7320 4C69 6365 6E73 652C 2074 6865"            /* his License, the */
	$"2072 6967 6874 7320 616E 6420 6C69 6365"            /*  rights and lice */
	$"6E73 6573 2067 7261 6E74 6564 2074 6F20"            /* nses granted to  */
	$"596F 7520 756E 6465 7220 7468 6973 204C"            /* You under this L */
	$"6963 656E 7365 2073 6861 6C6C 2069 6D6D"            /* icense shall imm */
	$"6564 6961 7465 6C79 2074 6572 6D69 6E61"            /* ediately termina */
	$"7465 2C20 616E 6420 596F 7520 7368 616C"            /* te, and You shal */
	$"6C20 696D 6D65 6469 6174 656C 7920 6365"            /* l immediately ce */
	$"6173 6520 7573 696E 672C 2061 6E64 2077"            /* ase using, and w */
	$"696C 6C20 7265 7475 726E 2074 6F20 414E"            /* ill return to AN */
	$"5343 4120 286F 722C 2061 7420 414E 5343"            /* SCA (or, at ANSC */
	$"4127 7320 7265 7175 6573 742C 2064 6573"            /* A's request, des */
	$"7472 6F79 292C 2074 6865 2044 6576 656C"            /* troy), the Devel */
	$"6F70 6572 2053 6F66 7477 6172 6520 7468"            /* oper Software th */
	$"6174 2061 7265 2070 726F 7072 6965 7461"            /* at are proprieta */
	$"7279 2074 6F20 6F72 2063 6F6E 7461 696E"            /* ry to or contain */
	$"2043 6F6E 6669 6465 6E74 6961 6C20 496E"            /*  Confidential In */
	$"666F 726D 6174 696F 6E2E 2054 6865 2072"            /* formation. The r */
	$"6967 6874 7320 616E 6420 6F62 6C69 6761"            /* ights and obliga */
	$"7469 6F6E 7320 6F66 2074 6865 2070 6172"            /* tions of the par */
	$"7469 6573 2073 6574 2066 6F72 7468 2069"            /* ties set forth i */
	$"6E20 5365 6374 696F 6E73 2032 4128 6969"            /* n Sections 2A(ii */
	$"6929 2C20 3243 2C20 3244 2C20 3245 2C20"            /* i), 2C, 2D, 2E,  */
	$"3246 2C20 332C 2034 2C20 352C 2036 2C20"            /* 2F, 3, 4, 5, 6,  */
	$"3720 616E 6420 3820 7368 616C 6C20 7375"            /* 7 and 8 shall su */
	$"7276 6976 6520 7465 726D 696E 6174 696F"            /* rvive terminatio */
	$"6E20 6F72 2065 7870 6972 6174 696F 6E20"            /* n or expiration  */
	$"6F66 2074 6869 7320 4C69 6365 6E73 6520"            /* of this License  */
	$"666F 7220 616E 7920 7265 6173 6F6E 2E0A"            /* for any reason.Â */
	$"0A42 2E20 5465 726D 696E 6174 696F 6E2E"            /* ÂB. Termination. */
	$"2054 6869 7320 4C69 6365 6E73 6520 6973"            /*  This License is */
	$"2065 6666 6563 7469 7665 2075 6E74 696C"            /*  effective until */
	$"2074 6572 6D69 6E61 7465 642E 2059 6F75"            /*  terminated. You */
	$"7220 7269 6768 7473 2075 6E64 6572 2074"            /* r rights under t */
	$"6869 7320 4C69 6365 6E73 6520 7769 6C6C"            /* his License will */
	$"2074 6572 6D69 6E61 7465 2061 7574 6F6D"            /*  terminate autom */
	$"6174 6963 616C 6C79 2077 6974 686F 7574"            /* atically without */
	$"206E 6F74 6963 6520 6672 6F6D 2041 4E53"            /*  notice from ANS */
	$"4341 2069 6620 596F 7520 6661 696C 2074"            /* CA if You fail t */
	$"6F20 636F 6D70 6C79 2077 6974 6820 616E"            /* o comply with an */
	$"7920 7465 726D 2873 2920 6F66 2074 6869"            /* y term(s) of thi */
	$"7320 4C69 6365 6E73 652E 2049 6E20 6164"            /* s License. In ad */
	$"6469 7469 6F6E 2C20 414E 5343 4120 7265"            /* dition, ANSCA re */
	$"7365 7276 6573 2074 6865 2072 6967 6874"            /* serves the right */
	$"2074 6F20 7465 726D 696E 6174 6520 7468"            /*  to terminate th */
	$"6973 204C 6963 656E 7365 2069 6620 6120"            /* is License if a  */
	$"6E65 7720 7665 7273 696F 6E20 6F66 2074"            /* new version of t */
	$"6865 2044 6576 656C 6F70 6572 2053 6F66"            /* he Developer Sof */
	$"7477 6172 6520 6973 2072 656C 6561 7365"            /* tware is release */
	$"6420 7768 6963 6820 6973 2069 6E63 6F6D"            /* d which is incom */
	$"7061 7469 626C 6520 7769 7468 2074 6869"            /* patible with thi */
	$"7320 7665 7273 696F 6E20 6F66 2074 6865"            /* s version of the */
	$"2044 6576 656C 6F70 6572 2053 6F66 7477"            /*  Developer Softw */
	$"6172 652E 2055 706F 6E20 7468 6520 7465"            /* are. Upon the te */
	$"726D 696E 6174 696F 6E20 6F66 2074 6869"            /* rmination of thi */
	$"7320 4C69 6365 6E73 652C 2059 6F75 2073"            /* s License, You s */
	$"6861 6C6C 2063 6561 7365 2061 6C6C 2075"            /* hall cease all u */
	$"7365 206F 6620 7468 6520 4465 7665 6C6F"            /* se of the Develo */
	$"7065 7220 536F 6674 7761 7265 2061 6E64"            /* per Software and */
	$"2064 6573 7472 6F79 2061 6C6C 2063 6F70"            /*  destroy all cop */
	$"6965 732C 2066 756C 6C20 6F72 2070 6172"            /* ies, full or par */
	$"7469 616C 2C20 6F66 2074 6865 2044 6576"            /* tial, of the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 652E"            /* eloper Software. */
	$"0A0A 5468 6973 2041 6772 6565 6D65 6E74"            /* ÂÂThis Agreement */
	$"2061 6E64 2061 6C6C 2072 6967 6874 7320"            /*  and all rights  */
	$"616E 6420 6C69 6365 6E73 6573 2067 7261"            /* and licenses gra */
	$"6E74 6564 2062 7920 414E 5343 4120 6865"            /* nted by ANSCA he */
	$"7265 756E 6465 7220 616E 6420 616E 7920"            /* reunder and any  */
	$"7365 7276 6963 6573 2070 726F 7669 6465"            /* services provide */
	$"6420 6865 7265 756E 6465 7220 7769 6C6C"            /* d hereunder will */
	$"2074 6572 6D69 6E61 7465 2C20 6566 6665"            /*  terminate, effe */
	$"6374 6976 6520 696D 6D65 6469 6174 656C"            /* ctive immediatel */
	$"7920 7570 6F6E 206E 6F74 6963 6520 6672"            /* y upon notice fr */
	$"6F6D 2041 4E53 4341 3ACA 0A28 6129 2069"            /* om ANSCA:ÊÂ(a) i */
	$"6620 596F 7520 6F72 2061 6E79 206F 6620"            /* f You or any of  */
	$"596F 7572 2041 7574 686F 7269 7A65 6420"            /* Your Authorized  */
	$"4465 7665 6C6F 7065 7273 2066 6169 6C20"            /* Developers fail  */
	$"746F 2063 6F6D 706C 7920 7769 7468 2061"            /* to comply with a */
	$"6E79 2074 6572 6D20 6F66 2074 6869 7320"            /* ny term of this  */
	$"4167 7265 656D 656E 7420 6F74 6865 7220"            /* Agreement other  */
	$"7468 616E 2074 686F 7365 2063 6F6E 7461"            /* than those conta */
	$"696E 6564 2069 6E20 5365 6374 696F 6E20"            /* ined in Section  */
	$"3520 2843 6F6E 6669 6465 6E74 6961 6C69"            /* 5 (Confidentiali */
	$"7479 2920 616E 6420 6661 696C 2074 6F20"            /* ty) and fail to  */
	$"6375 7265 2073 7563 6820 6272 6561 6368"            /* cure such breach */
	$"2077 6974 6869 6E20 3330 2064 6179 7320"            /*  within 30 days  */
	$"6166 7465 7220 6265 636F 6D69 6E67 2061"            /* after becoming a */
	$"7761 7265 206F 6620 6F72 2072 6563 6569"            /* ware of or recei */
	$"7669 6E67 206E 6F74 6963 6520 6F66 2073"            /* ving notice of s */
	$"7563 6820 6272 6561 6368 3BCA 0A28 6229"            /* uch breach;ÊÂ(b) */
	$"2069 6620 596F 7520 6F72 2061 6E79 206F"            /*  if You or any o */
	$"6620 596F 7572 2041 7574 686F 7269 7A65"            /* f Your Authorize */
	$"6420 4465 7665 6C6F 7065 7273 2066 6169"            /* d Developers fai */
	$"6C20 746F 2063 6F6D 706C 7920 7769 7468"            /* l to comply with */
	$"2074 6865 2074 6572 6D73 206F 6620 5365"            /*  the terms of Se */
	$"6374 696F 6E20 353B CA0A 2863 2920 696E"            /* ction 5;ÊÂ(c) in */
	$"2074 6865 2065 7665 6E74 206F 6620 7468"            /*  the event of th */
	$"6520 6369 7263 756D 7374 616E 6365 7320"            /* e circumstances  */
	$"6465 7363 7269 6265 6420 696E 2074 6865"            /* described in the */
	$"2073 7562 7365 6374 696F 6E20 656E 7469"            /*  subsection enti */
	$"746C 6564 2022 5365 7665 7261 6269 6C69"            /* tled "Severabili */
	$"7479 2220 6265 6C6F 773B CA0A 2864 2920"            /* ty" below;ÊÂ(d)  */
	$"6966 2059 6F75 2C20 6174 2061 6E79 2074"            /* if You, at any t */
	$"696D 6520 6475 7269 6E67 2074 6865 2054"            /* ime during the T */
	$"6572 6D2C 2063 6F6D 6D65 6E63 6520 616E"            /* erm, commence an */
	$"2061 6374 696F 6E20 666F 7220 7061 7465"            /*  action for pate */
	$"6E74 2069 6E66 7269 6E67 656D 656E 7420"            /* nt infringement  */
	$"6167 6169 6E73 7420 414E 5343 413B CA0A"            /* against ANSCA;ÊÂ */
	$"2865 2920 6966 2059 6F75 2062 6563 6F6D"            /* (e) if You becom */
	$"6520 696E 736F 6C76 656E 742C 2066 6169"            /* e insolvent, fai */
	$"6C20 746F 2070 6179 2059 6F75 7220 6465"            /* l to pay Your de */
	$"6274 7320 7768 656E 2064 7565 2C20 6469"            /* bts when due, di */
	$"7373 6F6C 7665 206F 7220 6365 6173 6520"            /* ssolve or cease  */
	$"746F 2064 6F20 6275 7369 6E65 7373 2C20"            /* to do business,  */
	$"6669 6C65 2066 6F72 2062 616E 6B72 7570"            /* file for bankrup */
	$"7463 792C 206F 7220 6861 7665 2066 696C"            /* tcy, or have fil */
	$"6564 2061 6761 696E 7374 2059 6F75 2061"            /* ed against You a */
	$"2070 6574 6974 696F 6E20 696E 2062 616E"            /*  petition in ban */
	$"6B72 7570 7463 793B 206F 72CA 0A28 6629"            /* kruptcy; orÊÂ(f) */
	$"2069 6620 596F 7520 656E 6761 6765 2C20"            /*  if You engage,  */
	$"6F72 2065 6E63 6F75 7261 6765 206F 7468"            /* or encourage oth */
	$"6572 7320 746F 2065 6E67 6167 652C 2069"            /* ers to engage, i */
	$"6E20 616E 7920 6672 6175 6475 6C65 6E74"            /* n any fraudulent */
	$"2C20 696D 7072 6F70 6572 2C20 756E 6C61"            /* , improper, unla */
	$"7766 756C 206F 7220 6469 7368 6F6E 6573"            /* wful or dishones */
	$"7420 6163 7420 7265 6C61 7469 6E67 2074"            /* t act relating t */
	$"6F20 7468 6973 2041 6772 6565 6D65 6E74"            /* o this Agreement */
	$"2C20 696E 636C 7564 696E 672C 2062 7574"            /* , including, but */
	$"206E 6F74 206C 696D 6974 6564 2074 6F2C"            /*  not limited to, */
	$"2065 6D62 657A 7A6C 656D 656E 742C 2061"            /*  embezzlement, a */
	$"6C74 6572 6174 696F 6E20 6F72 2066 616C"            /* lteration or fal */
	$"7369 6669 6361 7469 6F6E 206F 6620 646F"            /* sification of do */
	$"6375 6D65 6E74 732C 2074 6865 6674 2C20"            /* cuments, theft,  */
	$"696E 6170 7072 6F70 7269 6174 6520 7573"            /* inappropriate us */
	$"6520 6F66 2063 6F6D 7075 7465 7220 7379"            /* e of computer sy */
	$"7374 656D 732C 2062 7269 6265 7279 2C20"            /* stems, bribery,  */
	$"6F72 206F 7468 6572 206D 6973 7265 7072"            /* or other misrepr */
	$"6573 656E 7461 7469 6F6E 206F 6620 6661"            /* esentation of fa */
	$"6374 732E 2041 4E53 4341 206D 6179 2061"            /* cts. ANSCA may a */
	$"6C73 6F20 7465 726D 696E 6174 6520 7468"            /* lso terminate th */
	$"6973 2041 6772 6565 6D65 6E74 2C20 6F72"            /* is Agreement, or */
	$"2073 7573 7065 6E64 2059 6F75 7220 7269"            /*  suspend Your ri */
	$"6768 7473 2074 6F20 7573 6520 7468 6520"            /* ghts to use the  */
	$"414E 5343 4120 5344 4B20 6F72 2073 6572"            /* ANSCA SDK or ser */
	$"7669 6365 732C 2069 6620 596F 7520 6661"            /* vices, if You fa */
	$"696C 2074 6F20 6163 6365 7074 2061 6E79"            /* il to accept any */
	$"206E 6577 2052 6571 7569 7265 6D65 6E74"            /*  new Requirement */
	$"7320 6F72 2041 6772 6565 6D65 6E74 2074"            /* s or Agreement t */
	$"6572 6D73 2061 7320 6465 7363 7269 6265"            /* erms as describe */
	$"6420 696E 2053 6563 7469 6F6E 2035 432E"            /* d in Section 5C. */
	$"2045 6974 6865 7220 7061 7274 7920 6D61"            /*  Either party ma */
	$"7920 7465 726D 696E 6174 6520 7468 6973"            /* y terminate this */
	$"2041 6772 6565 6D65 6E74 2066 6F72 2069"            /*  Agreement for i */
	$"7473 2063 6F6E 7665 6E69 656E 6365 2C20"            /* ts convenience,  */
	$"666F 7220 616E 7920 7265 6173 6F6E 206F"            /* for any reason o */
	$"7220 6E6F 2072 6561 736F 6E2C 2065 6666"            /* r no reason, eff */
	$"6563 7469 7665 2033 3020 6461 7973 2061"            /* ective 30 days a */
	$"6674 6572 2070 726F 7669 6469 6E67 2074"            /* fter providing t */
	$"6865 206F 7468 6572 2070 6172 7479 2077"            /* he other party w */
	$"6974 6820 7772 6974 7465 6E20 6E6F 7469"            /* ith written noti */
	$"6365 206F 6620 6974 7320 696E 7465 6E74"            /* ce of its intent */
	$"2074 6F20 7465 726D 696E 6174 652E 0A0A"            /*  to terminate.ÂÂ */
	$"432E 2043 6861 6E67 6573 2074 6F20 5465"            /* C. Changes to Te */
	$"726D 732E 2041 4E53 4341 206D 6179 2063"            /* rms. ANSCA may c */
	$"6861 6E67 6520 7468 6520 7465 726D 7320"            /* hange the terms  */
	$"616E 6420 636F 6E64 6974 696F 6E73 206F"            /* and conditions o */
	$"6620 7468 6973 204C 6963 656E 7365 2061"            /* f this License a */
	$"7420 616E 7920 7469 6D65 2E20 4E65 7720"            /* t any time. New  */
	$"6F72 206D 6F64 6966 6965 6420 7465 726D"            /* or modified term */
	$"7320 616E 6420 636F 6E64 6974 696F 6E73"            /* s and conditions */
	$"2077 696C 6C20 6E6F 7420 7265 7472 6F61"            /*  will not retroa */
	$"6374 6976 656C 7920 6170 706C 7920 746F"            /* ctively apply to */
	$"2041 7070 6C69 6361 7469 6F6E 7320 6372"            /*  Applications cr */
	$"6561 7465 6420 7769 7468 2074 6865 2044"            /* eated with the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 616C 7265 6164 7920 696E 2064 6973"            /* e already in dis */
	$"7472 6962 7574 696F 6E2E 2049 6E20 6F72"            /* tribution. In or */
	$"6465 7220 746F 2063 6F6E 7469 6E75 6520"            /* der to continue  */
	$"7573 696E 6720 7468 6520 4465 7665 6C6F"            /* using the Develo */
	$"7065 7220 536F 6674 7761 7265 2C20 596F"            /* per Software, Yo */
	$"7520 6D75 7374 2061 6363 6570 7420 616E"            /* u must accept an */
	$"6420 6167 7265 6520 746F 2074 6865 206E"            /* d agree to the n */
	$"6577 2074 6572 6D73 2061 6E64 2063 6F6E"            /* ew terms and con */
	$"6469 7469 6F6E 7320 6F66 2074 6869 7320"            /* ditions of this  */
	$"4C69 6365 6E73 652E 2049 6620 596F 7520"            /* License. If You  */
	$"646F 206E 6F74 2061 6772 6565 2074 6F20"            /* do not agree to  */
	$"7468 6520 6E65 7720 7465 726D 7320 616E"            /* the new terms an */
	$"6420 636F 6E64 6974 696F 6E73 2C20 596F"            /* d conditions, Yo */
	$"7572 2075 7365 206F 6620 7468 6520 4465"            /* ur use of the De */
	$"7665 6C6F 7065 7220 536F 6674 7761 7265"            /* veloper Software */
	$"2077 696C 6C20 6265 2073 7573 7065 6E64"            /*  will be suspend */
	$"6564 206F 7220 7465 726D 696E 6174 6564"            /* ed or terminated */
	$"2062 7920 414E 5343 412E 2059 6F75 2061"            /*  by ANSCA. You a */
	$"6772 6565 2074 6861 7420 596F 7572 2061"            /* gree that Your a */
	$"6363 6570 7461 6E63 6520 6F66 2073 7563"            /* cceptance of suc */
	$"6820 6E65 7720 4C69 6365 6E73 6520 7465"            /* h new License te */
	$"726D 7320 616E 6420 636F 6E64 6974 696F"            /* rms and conditio */
	$"6E73 206D 6179 2062 6520 7369 676E 6966"            /* ns may be signif */
	$"6965 6420 656C 6563 7472 6F6E 6963 616C"            /* ied electronical */
	$"6C79 2C20 696E 636C 7564 696E 6720 7769"            /* ly, including wi */
	$"7468 6F75 7420 6C69 6D69 7461 7469 6F6E"            /* thout limitation */
	$"2C20 6279 2059 6F75 7220 6368 6563 6B69"            /* , by Your checki */
	$"6E67 2061 2062 6F78 206F 7220 636C 6963"            /* ng a box or clic */
	$"6B69 6E67 206F 6E20 616E 2022 6167 7265"            /* king on an "agre */
	$"6522 206F 7220 7369 6D69 6C61 7220 6275"            /* e" or similar bu */
	$"7474 6F6E 2E20 4E6F 7468 696E 6720 696E"            /* tton. Nothing in */
	$"2074 6869 7320 5365 6374 696F 6E20 7368"            /*  this Section sh */
	$"616C 6C20 6166 6665 6374 2041 4E53 4341"            /* all affect ANSCA */
	$"2773 2072 6967 6874 7320 756E 6465 7220"            /* 's rights under  */
	$"5365 6374 696F 6E20 3544 2062 656C 6F77"            /* Section 5D below */
	$"2E0A 0A44 2E20 5265 766F 6361 7469 6F6E"            /* .ÂÂD. Revocation */
	$"2E20 596F 7520 756E 6465 7273 7461 6E64"            /* . You understand */
	$"2061 6E64 2061 6772 6565 2074 6861 7420"            /*  and agree that  */
	$"414E 5343 4120 6D61 7920 6365 6173 6520"            /* ANSCA may cease  */
	$"596F 7572 2061 6269 6C69 7479 2074 6F20"            /* Your ability to  */
	$"6372 6561 7465 2041 7070 6C69 6361 7469"            /* create Applicati */
	$"6F6E 7320 6F72 206F 7468 6572 2073 6F66"            /* ons or other sof */
	$"7477 6172 6520 7573 696E 6720 7468 6520"            /* tware using the  */
	$"5344 4B20 6174 2061 6E79 2074 696D 652E"            /* SDK at any time. */
	$"2042 7920 7761 7920 6F66 2065 7861 6D70"            /*  By way of examp */
	$"6C65 206F 6E6C 792C 2041 4E53 4341 206D"            /* le only, ANSCA m */
	$"6967 6874 2063 686F 6F73 6520 746F 2064"            /* ight choose to d */
	$"6F20 7468 6973 2069 6620 6174 2061 6E79"            /* o this if at any */
	$"2074 696D 653A CA0A 2861 2920 414E 5343"            /*  time:ÊÂ(a) ANSC */
	$"4120 6861 7320 6265 656E 206E 6F74 6966"            /* A has been notif */
	$"6965 6420 6F72 206F 7468 6572 7769 7365"            /* ied or otherwise */
	$"2068 6173 2072 6561 736F 6E20 746F 2062"            /*  has reason to b */
	$"656C 6965 7665 2074 6861 7420 596F 7572"            /* elieve that Your */
	$"2041 7070 6C69 6361 7469 6F6E 2076 696F"            /*  Application vio */
	$"6C61 7465 732C 206D 6973 6170 7072 6F70"            /* lates, misapprop */
	$"7269 6174 6573 2C20 6F72 2069 6E66 7269"            /* riates, or infri */
	$"6E67 6573 2074 6865 2072 6967 6874 7320"            /* nges the rights  */
	$"6F66 2061 2074 6869 7264 2070 6172 7479"            /* of a third party */
	$"206F 7220 6F66 2041 4E53 4341 3BCA 0A28"            /*  or of ANSCA;ÊÂ( */
	$"6229 2041 4E53 4341 2068 6173 2072 6561"            /* b) ANSCA has rea */
	$"736F 6E20 746F 2062 656C 6965 7665 2074"            /* son to believe t */
	$"6861 7420 596F 7520 6861 7665 206F 7220"            /* hat You have or  */
	$"7769 6C6C 2063 7265 6174 6520 4170 706C"            /* will create Appl */
	$"6963 6174 696F 6E73 2075 7369 6E67 2074"            /* ications using t */
	$"6865 2053 444B 2074 6861 7420 636F 6E74"            /* he SDK that cont */
	$"6169 6E20 6D61 6C69 6369 6F75 7320 6F72"            /* ain malicious or */
	$"2068 6172 6D66 756C 2063 6F64 652C 206D"            /*  harmful code, m */
	$"616C 7761 7265 2C20 7072 6F67 7261 6D73"            /* alware, programs */
	$"206F 7220 6F74 6865 7220 696E 7465 726E"            /*  or other intern */
	$"616C 2063 6F6D 706F 6E65 6E74 7320 2865"            /* al components (e */
	$"2E67 2E20 736F 6674 7761 7265 2076 6972"            /* .g. software vir */
	$"7573 293B CA0A 2863 2920 414E 5343 4120"            /* us);ÊÂ(c) ANSCA  */
	$"6861 7320 7265 6173 6F6E 2074 6F20 6265"            /* has reason to be */
	$"6C69 6576 6520 7468 6174 2059 6F75 7220"            /* lieve that Your  */
	$"6170 706C 6963 6174 696F 6E20 6461 6D61"            /* application dama */
	$"6765 732C 2063 6F72 7275 7074 732C 2064"            /* ges, corrupts, d */
	$"6567 7261 6465 732C 2064 6573 7472 6F79"            /* egrades, destroy */
	$"7320 6F72 206F 7468 6572 7769 7365 2061"            /* s or otherwise a */
	$"6476 6572 7365 6C79 2061 6666 6563 7473"            /* dversely affects */
	$"2074 6865 2064 6576 6963 6573 2069 7420"            /*  the devices it  */
	$"6F70 6572 6174 6573 206F 6E2C 206F 7220"            /* operates on, or  */
	$"616E 79CA 0A6F 7468 6572 2073 6F66 7477"            /* anyÊÂother softw */
	$"6172 652C 2066 6972 6D77 6172 652C 2068"            /* are, firmware, h */
	$"6172 6477 6172 652C 2064 6174 612C 2073"            /* ardware, data, s */
	$"7973 7465 6D73 2C20 6F72 206E 6574 776F"            /* ystems, or netwo */
	$"726B 7320 6163 6365 7373 6564 206F 7220"            /* rks accessed or  */
	$"7573 6564 2062 7920 7468 6520 6170 706C"            /* used by the appl */
	$"6963 6174 696F 6E3B CA0A 2864 2920 596F"            /* ication;ÊÂ(d) Yo */
	$"7520 6272 6561 6368 2061 6E79 2074 6572"            /* u breach any ter */
	$"6D20 6F72 2063 6F6E 6469 7469 6F6E 206F"            /* m or condition o */
	$"6620 7468 6973 204C 6963 656E 7365 3B0A"            /* f this License;Â */
	$"2865 2920 416E 7920 696E 666F 726D 6174"            /* (e) Any informat */
	$"696F 6E20 6F72 2064 6F63 756D 656E 7473"            /* ion or documents */
	$"2070 726F 7669 6465 6420 6279 2059 6F75"            /*  provided by You */
	$"2074 6F20 414E 5343 4120 666F 7220 7468"            /*  to ANSCA for th */
	$"6520 7075 7270 6F73 6520 6F66 2076 6572"            /* e purpose of ver */
	$"6966 7969 6E67 2059 6F75 7220 6964 656E"            /* ifying Your iden */
	$"7469 7479 206F 7220 6F62 7461 696E 696E"            /* tity or obtainin */
	$"6720 414E 5343 412D 6973 7375 6564 2064"            /* g ANSCA-issued d */
	$"6967 6974 616C 2063 6572 7469 6669 6361"            /* igital certifica */
	$"7465 7320 6973 2066 616C 7365 206F 7220"            /* tes is false or  */
	$"696E 6163 6375 7261 7465 3BCA 0A28 6629"            /* inaccurate;ÊÂ(f) */
	$"2041 6E79 2072 6570 7265 7365 6E74 6174"            /*  Any representat */
	$"696F 6E2C 2077 6172 7261 6E74 7920 6F72"            /* ion, warranty or */
	$"2063 6572 7469 6669 6361 7469 6F6E 2070"            /*  certification p */
	$"726F 7669 6465 6420 6279 2059 6F75 2074"            /* rovided by You t */
	$"6F20 414E 5343 4120 696E 2074 6869 7320"            /* o ANSCA in this  */
	$"4C69 6365 6E73 6520 6973 2075 6E74 7275"            /* License is untru */
	$"6520 6F72 2069 6E61 6363 7572 6174 653B"            /* e or inaccurate; */
	$"CA0A 2867 2920 414E 5343 4120 6973 2072"            /* ÊÂ(g) ANSCA is r */
	$"6571 7569 7265 6420 6279 206C 6177 2C20"            /* equired by law,  */
	$"7265 6775 6C61 7469 6F6E 206F 7220 6F74"            /* regulation or ot */
	$"6865 7220 676F 7665 726E 6D65 6E74 616C"            /* her governmental */
	$"206F 7220 636F 7572 7420 6F72 6465 7220"            /*  or court order  */
	$"746F 2074 616B 6520 7375 6368 2061 6374"            /* to take such act */
	$"696F 6E3B CA0A 2868 2920 414E 5343 4120"            /* ion;ÊÂ(h) ANSCA  */
	$"6861 7320 7265 6173 6F6E 2074 6F20 6265"            /* has reason to be */
	$"6C69 6576 6520 7468 6174 2073 7563 6820"            /* lieve that such  */
	$"6163 7469 6F6E 2069 7320 7072 7564 656E"            /* action is pruden */
	$"7420 6F72 206E 6563 6573 7361 7279 2E0A"            /* t or necessary.Â */
	$"0A0A 352E 2043 6F6E 6669 6465 6E74 6961"            /* ÂÂ5. Confidentia */
	$"6C69 7479 2E20 2243 6F6E 6669 6465 6E74"            /* lity. "Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E22"            /* ial Information" */
	$"2073 6861 6C6C 206D 6561 6E20 616C 6C20"            /*  shall mean all  */
	$"7472 6164 6520 7365 6372 6574 732C 206B"            /* trade secrets, k */
	$"6E6F 772D 686F 772C 2069 6E76 656E 7469"            /* now-how, inventi */
	$"6F6E 732C 2074 6563 686E 6971 7565 732C"            /* ons, techniques, */
	$"2070 726F 6365 7373 6573 2C20 616C 676F"            /*  processes, algo */
	$"7269 7468 6D73 2C20 736F 6674 7761 7265"            /* rithms, software */
	$"2070 726F 6772 616D 732C 2068 6172 6477"            /*  programs, hardw */
	$"6172 652C 2073 6368 656D 6174 6963 732C"            /* are, schematics, */
	$"2061 6E64 2073 6F66 7477 6172 6520 736F"            /*  and software so */
	$"7572 6365 2064 6F63 756D 656E 7473 2072"            /* urce documents r */
	$"656C 6174 696E 6720 746F 2074 6865 2044"            /* elating to the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"652C 2061 6E64 206F 7468 6572 2069 6E66"            /* e, and other inf */
	$"6F72 6D61 7469 6F6E 2070 726F 7669 6465"            /* ormation provide */
	$"6420 6279 2041 4E53 4341 2C20 7768 6574"            /* d by ANSCA, whet */
	$"6865 7220 6469 7363 6C6F 7365 6420 6F72"            /* her disclosed or */
	$"616C 6C79 2C20 696E 2077 7269 7469 6E67"            /* ally, in writing */
	$"2C20 6F72 2062 7920 6578 616D 696E 6174"            /* , or by examinat */
	$"696F 6E20 6F72 2069 6E73 7065 6374 696F"            /* ion or inspectio */
	$"6E2C 206F 7468 6572 2074 6861 6E20 696E"            /* n, other than in */
	$"666F 726D 6174 696F 6E20 7768 6963 6820"            /* formation which  */
	$"596F 7520 6361 6E20 6465 6D6F 6E73 7472"            /* You can demonstr */
	$"6174 6520 2869 2920 7761 7320 616C 7265"            /* ate (i) was alre */
	$"6164 7920 6B6E 6F77 6E20 746F 2059 6F75"            /* ady known to You */
	$"2C20 6F74 6865 7220 7468 616E 2075 6E64"            /* , other than und */
	$"6572 2061 6E20 6F62 6C69 6761 7469 6F6E"            /* er an obligation */
	$"206F 6620 636F 6E66 6964 656E 7469 616C"            /*  of confidential */
	$"6974 792C 2061 7420 7468 6520 7469 6D65"            /* ity, at the time */
	$"206F 6620 6469 7363 6C6F 7375 7265 3B20"            /*  of disclosure;  */
	$"2869 6929 2077 6173 2067 656E 6572 616C"            /* (ii) was general */
	$"6C79 2061 7661 696C 6162 6C65 2069 6E20"            /* ly available in  */
	$"7468 6520 7075 626C 6963 2064 6F6D 6169"            /* the public domai */
	$"6E20 6174 2074 6865 2074 696D 6520 6F66"            /* n at the time of */
	$"2064 6973 636C 6F73 7572 6520 746F 2059"            /*  disclosure to Y */
	$"6F75 3B20 2869 6969 2920 6265 6361 6D65"            /* ou; (iii) became */
	$"2067 656E 6572 616C 6C79 2061 7661 696C"            /*  generally avail */
	$"6162 6C65 2069 6E20 7468 6520 7075 626C"            /* able in the publ */
	$"6963 2064 6F6D 6169 6E20 6166 7465 7220"            /* ic domain after  */
	$"6469 7363 6C6F 7375 7265 206F 7468 6572"            /* disclosure other */
	$"2074 6861 6E20 7468 726F 7567 6820 616E"            /*  than through an */
	$"7920 6163 7420 6F72 206F 6D69 7373 696F"            /* y act or omissio */
	$"6E20 6F66 2059 6F75 3B20 2869 7629 2077"            /* n of You; (iv) w */
	$"6173 2073 7562 7365 7175 656E 746C 7920"            /* as subsequently  */
	$"6C61 7766 756C 6C79 2064 6973 636C 6F73"            /* lawfully disclos */
	$"6564 2074 6F20 596F 7520 6279 2061 2074"            /* ed to You by a t */
	$"6869 7264 2070 6172 7479 2077 6974 686F"            /* hird party witho */
	$"7574 2061 6E79 206F 626C 6967 6174 696F"            /* ut any obligatio */
	$"6E20 6F66 2063 6F6E 6669 6465 6E74 6961"            /* n of confidentia */
	$"6C69 7479 3B20 6F72 2028 7629 2077 6173"            /* lity; or (v) was */
	$"2069 6E64 6570 656E 6465 6E74 6C79 2064"            /*  independently d */
	$"6576 656C 6F70 6564 2062 7920 596F 7520"            /* eveloped by You  */
	$"7769 7468 6F75 7420 7573 6520 6F66 206F"            /* without use of o */
	$"7220 7265 6665 7265 6E63 6520 746F 2061"            /* r reference to a */
	$"6E79 2069 6E66 6F72 6D61 7469 6F6E 206F"            /* ny information o */
	$"7220 6D61 7465 7269 616C 7320 6469 7363"            /* r materials disc */
	$"6C6F 7365 6420 6279 2041 4E53 4341 206F"            /* losed by ANSCA o */
	$"7220 6974 7320 7375 7070 6C69 6572 732E"            /* r its suppliers. */
	$"2043 6F6E 6669 6465 6E74 6961 6C20 496E"            /*  Confidential In */
	$"666F 726D 6174 696F 6E20 7368 616C 6C20"            /* formation shall  */
	$"696E 636C 7564 6520 7769 7468 6F75 7420"            /* include without  */
	$"6C69 6D69 7461 7469 6F6E 2074 6865 2044"            /* limitation the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"652C 2050 6572 666F 726D 616E 6365 2044"            /* e, Performance D */
	$"6174 612C 2061 6E64 2061 6E79 2075 7064"            /* ata, and any upd */
	$"6174 6573 2E20 596F 7520 7368 616C 6C20"            /* ates. You shall  */
	$"6E6F 7420 7573 6520 616E 7920 436F 6E66"            /* not use any Conf */
	$"6964 656E 7469 616C 2049 6E66 6F72 6D61"            /* idential Informa */
	$"7469 6F6E 2066 6F72 2061 6E79 2070 7572"            /* tion for any pur */
	$"706F 7365 206F 7468 6572 2074 6861 6E20"            /* pose other than  */
	$"6173 2065 7870 7265 7373 6C79 2061 7574"            /* as expressly aut */
	$"686F 7269 7A65 6420 756E 6465 7220 7468"            /* horized under th */
	$"6973 204C 6963 656E 7365 2E20 496E 206E"            /* is License. In n */
	$"6F20 6576 656E 7420 7368 616C 6C20 596F"            /* o event shall Yo */
	$"7520 7573 6520 7468 6520 4465 7665 6C6F"            /* u use the Develo */
	$"7065 7220 536F 6674 7761 7265 206F 7220"            /* per Software or  */
	$"616E 7920 436F 6E66 6964 656E 7469 616C"            /* any Confidential */
	$"2049 6E66 6F72 6D61 7469 6F6E 2074 6F20"            /*  Information to  */
	$"6465 7665 6C6F 702C 206D 616E 7566 6163"            /* develop, manufac */
	$"7475 7265 2C20 6D61 726B 6574 2C20 7365"            /* ture, market, se */
	$"6C6C 2C20 6F72 2064 6973 7472 6962 7574"            /* ll, or distribut */
	$"6520 616E 7920 7072 6F64 7563 7420 6F72"            /* e any product or */
	$"2073 6572 7669 6365 2E20 596F 7520 7368"            /*  service. You sh */
	$"616C 6C20 6C69 6D69 7420 6469 7373 656D"            /* all limit dissem */
	$"696E 6174 696F 6E20 6F66 2043 6F6E 6669"            /* ination of Confi */
	$"6465 6E74 6961 6C20 496E 666F 726D 6174"            /* dential Informat */
	$"696F 6E20 746F 2069 7473 2065 6D70 6C6F"            /* ion to its emplo */
	$"7965 6573 2077 686F 2068 6176 6520 6120"            /* yees who have a  */
	$"6E65 6564 2074 6F20 6B6E 6F77 2073 7563"            /* need to know suc */
	$"6820 436F 6E66 6964 656E 7469 616C 2049"            /* h Confidential I */
	$"6E66 6F72 6D61 7469 6F6E 2066 6F72 2070"            /* nformation for p */
	$"7572 706F 7365 7320 6578 7072 6573 736C"            /* urposes expressl */
	$"7920 6175 7468 6F72 697A 6564 2075 6E64"            /* y authorized und */
	$"6572 2074 6869 7320 4C69 6365 6E73 652E"            /* er this License. */
	$"2049 6E20 6E6F 2065 7665 6E74 2073 6861"            /*  In no event sha */
	$"6C6C 2059 6F75 2064 6973 636C 6F73 6520"            /* ll You disclose  */
	$"616E 7920 436F 6E66 6964 656E 7469 616C"            /* any Confidential */
	$"2049 6E66 6F72 6D61 7469 6F6E 2074 6F20"            /*  Information to  */
	$"616E 7920 7468 6972 6420 7061 7274 792E"            /* any third party. */
	$"2057 6974 686F 7574 206C 696D 6974 696E"            /*  Without limitin */
	$"6720 7468 6520 666F 7265 676F 696E 672C"            /* g the foregoing, */
	$"2059 6F75 2073 6861 6C6C 2075 7365 2061"            /*  You shall use a */
	$"7420 6C65 6173 7420 7468 6520 7361 6D65"            /* t least the same */
	$"2064 6567 7265 6520 6F66 2063 6172 6520"            /*  degree of care  */
	$"7468 6174 2069 7420 7573 6573 2074 6F20"            /* that it uses to  */
	$"7072 6576 656E 7420 7468 6520 6469 7363"            /* prevent the disc */
	$"6C6F 7375 7265 206F 6620 6974 7320 6F77"            /* losure of its ow */
	$"6E20 636F 6E66 6964 656E 7469 616C 2069"            /* n confidential i */
	$"6E66 6F72 6D61 7469 6F6E 206F 6620 6C69"            /* nformation of li */
	$"6B65 2069 6D70 6F72 7461 6E63 652C 2062"            /* ke importance, b */
	$"7574 2069 6E20 6E6F 2065 7665 6E74 206C"            /* ut in no event l */
	$"6573 7320 7468 616E 2072 6561 736F 6E61"            /* ess than reasona */
	$"626C 6520 6361 7265 2C20 746F 2070 7265"            /* ble care, to pre */
	$"7665 6E74 2074 6865 2064 6973 636C 6F73"            /* vent the disclos */
	$"7572 6520 6F66 2043 6F6E 6669 6465 6E74"            /* ure of Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E2E"            /* ial Information. */
	$"0A0A 412E 204F 626C 6967 6174 696F 6E73"            /* ÂÂA. Obligations */
	$"2052 6567 6172 6469 6E67 2041 4E53 4341"            /*  Regarding ANSCA */
	$"2043 6F6E 6669 6465 6E74 6961 6C20 496E"            /*  Confidential In */
	$"666F 726D 6174 696F 6E2E 2059 6F75 2061"            /* formation. You a */
	$"6772 6565 2074 6F20 7072 6F74 6563 7420"            /* gree to protect  */
	$"416E 7363 6120 436F 6E66 6964 656E 7469"            /* Ansca Confidenti */
	$"616C 2049 6E66 6F72 6D61 7469 6F6E 2075"            /* al Information u */
	$"7369 6E67 2061 7420 6C65 6173 7420 7468"            /* sing at least th */
	$"6520 7361 6D65 2064 6567 7265 6520 6F66"            /* e same degree of */
	$"2063 6172 6520 7468 6174 2059 6F75 2075"            /*  care that You u */
	$"7365 2074 6F20 7072 6F74 6563 7420 596F"            /* se to protect Yo */
	$"7572 206F 776E 2063 6F6E 6669 6465 6E74"            /* ur own confident */
	$"6961 6C20 696E 666F 726D 6174 696F 6E20"            /* ial information  */
	$"6F66 2073 696D 696C 6172 2069 6D70 6F72"            /* of similar impor */
	$"7461 6E63 652C 2062 7574 206E 6F20 6C65"            /* tance, but no le */
	$"7373 2074 6861 6E20 6120 7265 6173 6F6E"            /* ss than a reason */
	$"6162 6C65 2064 6567 7265 6520 6F66 2063"            /* able degree of c */
	$"6172 652E 2059 6F75 2061 6772 6565 2074"            /* are. You agree t */
	$"6F20 7573 6520 414E 5343 4120 436F 6E66"            /* o use ANSCA Conf */
	$"6964 656E 7469 616C 2049 6E66 6F72 6D61"            /* idential Informa */
	$"7469 6F6E 2073 6F6C 656C 7920 666F 7220"            /* tion solely for  */
	$"7468 6520 7075 7270 6F73 6520 6F66 2065"            /* the purpose of e */
	$"7865 7263 6973 696E 6720 596F 7572 2072"            /* xercising Your r */
	$"6967 6874 7320 616E 6420 7065 7266 6F72"            /* ights and perfor */
	$"6D69 6E67 2059 6F75 7220 6F62 6C69 6761"            /* ming Your obliga */
	$"7469 6F6E 7320 756E 6465 7220 7468 6973"            /* tions under this */
	$"2041 6772 6565 6D65 6E74 2061 6E64 2061"            /*  Agreement and a */
	$"6772 6565 206E 6F74 2074 6F20 7573 6520"            /* gree not to use  */
	$"414E 5343 4120 436F 6E66 6964 656E 7469"            /* ANSCA Confidenti */
	$"616C 2049 6E66 6F72 6D61 7469 6F6E 2066"            /* al Information f */
	$"6F72 2061 6E79 206F 7468 6572 2070 7572"            /* or any other pur */
	$"706F 7365 2C20 666F 7220 596F 7572 206F"            /* pose, for Your o */
	$"776E 206F 7220 616E 7920 7468 6972 6420"            /* wn or any third  */
	$"7061 7274 7927 7320 6265 6E65 6669 742C"            /* party's benefit, */
	$"2077 6974 686F 7574 2041 4E53 4341 2773"            /*  without ANSCA's */
	$"2070 7269 6F72 2077 7269 7474 656E 2063"            /*  prior written c */
	$"6F6E 7365 6E74 2E20 596F 7520 6675 7274"            /* onsent. You furt */
	$"6865 7220 6167 7265 6520 6E6F 7420 746F"            /* her agree not to */
	$"2064 6973 636C 6F73 6520 6F72 2064 6973"            /*  disclose or dis */
	$"7365 6D69 6E61 7465 2041 4E53 4341 2043"            /* seminate ANSCA C */
	$"6F6E 6669 6465 6E74 6961 6C20 496E 666F"            /* onfidential Info */
	$"726D 6174 696F 6E20 746F 2061 6E79 6F6E"            /* rmation to anyon */
	$"6520 6F74 6865 7220 7468 616E 3A20 2869"            /* e other than: (i */
	$"2920 7468 6F73 6520 6F66 2059 6F75 7220"            /* ) those of Your  */
	$"656D 706C 6F79 6565 7320 616E 6420 636F"            /* employees and co */
	$"6E74 7261 6374 6F72 732C 206F 7220 7468"            /* ntractors, or th */
	$"6F73 6520 6F66 2059 6F75 7220 6661 6375"            /* ose of Your facu */
	$"6C74 7920 616E 6420 7374 6166 6620 6966"            /* lty and staff if */
	$"2059 6F75 2061 7265 2061 6E20 6564 7563"            /*  You are an educ */
	$"6174 696F 6E61 6C20 696E 7374 6974 7574"            /* ational institut */
	$"696F 6E2C 2077 686F 2068 6176 6520 6120"            /* ion, who have a  */
	$"6E65 6564 2074 6F20 6B6E 6F77 2061 6E64"            /* need to know and */
	$"2077 686F 2061 7265 2062 6F75 6E64 2062"            /*  who are bound b */
	$"7920 6120 7772 6974 7465 6E20 6167 7265"            /* y a written agre */
	$"656D 656E 7420 7468 6174 2070 726F 6869"            /* ement that prohi */
	$"6269 7473 2075 6E61 7574 686F 7269 7A65"            /* bits unauthorize */
	$"6420 7573 6520 6F72 2064 6973 636C 6F73"            /* d use or disclos */
	$"7572 6520 6F66 2074 6865 2041 4E53 4341"            /* ure of the ANSCA */
	$"2043 6F6E 6669 6465 6E74 6961 6C20 496E"            /*  Confidential In */
	$"666F 726D 6174 696F 6E3B 206F 7220 2869"            /* formation; or (i */
	$"6929 2065 7863 6570 7420 6173 206F 7468"            /* i) except as oth */
	$"6572 7769 7365 2061 6772 6565 6420 6F72"            /* erwise agreed or */
	$"2070 6572 6D69 7474 6564 2069 6E20 7772"            /*  permitted in wr */
	$"6974 696E 6720 6279 2041 4E53 4341 2E20"            /* iting by ANSCA.  */
	$"596F 7520 6D61 7920 6469 7363 6C6F 7365"            /* You may disclose */
	$"2041 4E53 4341 2043 6F6E 6669 6465 6E74"            /*  ANSCA Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E20"            /* ial Information  */
	$"746F 2074 6865 2065 7874 656E 7420 7265"            /* to the extent re */
	$"7175 6972 6564 2062 7920 6C61 772C 2070"            /* quired by law, p */
	$"726F 7669 6465 6420 7468 6174 2059 6F75"            /* rovided that You */
	$"2074 616B 6520 7265 6173 6F6E 6162 6C65"            /*  take reasonable */
	$"2073 7465 7073 2074 6F20 6E6F 7469 6679"            /*  steps to notify */
	$"2041 4E53 4341 206F 6620 7375 6368 2072"            /*  ANSCA of such r */
	$"6571 7569 7265 6D65 6E74 2062 6566 6F72"            /* equirement befor */
	$"6520 6469 7363 6C6F 7369 6E67 2074 6865"            /* e disclosing the */
	$"2041 4E53 4341 2043 6F6E 6669 6465 6E74"            /*  ANSCA Confident */
	$"6961 6C20 496E 666F 726D 6174 696F 6E20"            /* ial Information  */
	$"616E 6420 746F 206F 6274 6169 6E20 7072"            /* and to obtain pr */
	$"6F74 6563 7469 7665 2074 7265 6174 6D65"            /* otective treatme */
	$"6E74 206F 6620 7468 6520 414E 5343 4120"            /* nt of the ANSCA  */
	$"436F 6E66 6964 656E 7469 616C 2049 6E66"            /* Confidential Inf */
	$"6F72 6D61 7469 6F6E 2E20 596F 7520 6163"            /* ormation. You ac */
	$"6B6E 6F77 6C65 6467 6520 7468 6174 2064"            /* knowledge that d */
	$"616D 6167 6573 2066 6F72 2069 6D70 726F"            /* amages for impro */
	$"7065 7220 6469 7363 6C6F 7375 7265 206F"            /* per disclosure o */
	$"6620 414E 5343 4120 436F 6E66 6964 656E"            /* f ANSCA Confiden */
	$"7469 616C 2049 6E66 6F72 6D61 7469 6F6E"            /* tial Information */
	$"206D 6179 2062 6520 6972 7265 7061 7261"            /*  may be irrepara */
	$"626C 653B 2074 6865 7265 666F 7265 2C20"            /* ble; therefore,  */
	$"414E 5343 4120 6973 2065 6E74 6974 6C65"            /* ANSCA is entitle */
	$"6420 746F 2073 6565 6B20 6571 7569 7461"            /* d to seek equita */
	$"626C 6520 7265 6C69 6566 2C20 696E 636C"            /* ble relief, incl */
	$"7564 696E 6720 696E 6A75 6E63 7469 6F6E"            /* uding injunction */
	$"2061 6E64 2070 7265 6C69 6D69 6E61 7279"            /*  and preliminary */
	$"2069 6E6A 756E 6374 696F 6E2C 2069 6E20"            /*  injunction, in  */
	$"6164 6469 7469 6F6E 2074 6F20 616C 6C20"            /* addition to all  */
	$"6F74 6865 7220 7265 6D65 6469 6573 2ECA"            /* other remedies.Ê */
	$"0A0A 422E 2049 6E66 6F72 6D61 7469 6F6E"            /* ÂÂB. Information */
	$"2053 7562 6D69 7474 6564 2074 6F20 414E"            /*  Submitted to AN */
	$"5343 4120 4E6F 7420 4465 656D 6564 2043"            /* SCA Not Deemed C */
	$"6F6E 6669 6465 6E74 6961 6C2E 2041 4E53"            /* onfidential. ANS */
	$"4341 2077 6F72 6B73 2077 6974 6820 6D61"            /* CA works with ma */
	$"6E79 2061 7070 6C69 6361 7469 6F6E 2061"            /* ny application a */
	$"6E64 2073 6F66 7477 6172 6520 6465 7665"            /* nd software deve */
	$"6C6F 7065 7273 2061 6E64 2073 6F6D 6520"            /* lopers and some  */
	$"6F66 2074 6865 6972 2070 726F 6475 6374"            /* of their product */
	$"7320 6D61 7920 6265 2073 696D 696C 6172"            /* s may be similar */
	$"2074 6F20 6F72 2063 6F6D 7065 7465 2077"            /*  to or compete w */
	$"6974 6820 596F 7572 2041 7070 6C69 6361"            /* ith Your Applica */
	$"7469 6F6E 732E 2041 4E53 4341 206D 6179"            /* tions. ANSCA may */
	$"2061 6C73 6F20 6265 2064 6576 656C 6F70"            /*  also be develop */
	$"696E 6720 6974 7320 6F77 6E20 7369 6D69"            /* ing its own simi */
	$"6C61 7220 6F72 2063 6F6D 7065 7469 6E67"            /* lar or competing */
	$"2061 7070 6C69 6361 7469 6F6E 7320 616E"            /*  applications an */
	$"6420 7072 6F64 7563 7473 206F 7220 6D61"            /* d products or ma */
	$"7920 6465 6369 6465 2074 6F20 646F 2073"            /* y decide to do s */
	$"6F20 696E 2074 6865 2066 7574 7572 652E"            /* o in the future. */
	$"2054 6F20 6176 6F69 6420 706F 7465 6E74"            /*  To avoid potent */
	$"6961 6C20 6D69 7375 6E64 6572 7374 616E"            /* ial misunderstan */
	$"6469 6E67 732C 2041 4E53 4341 2063 616E"            /* dings, ANSCA can */
	$"6E6F 7420 6167 7265 652C 2061 6E64 2065"            /* not agree, and e */
	$"7870 7265 7373 6C79 2064 6973 636C 6169"            /* xpressly disclai */
	$"6D73 2C20 616E 7920 636F 6E66 6964 656E"            /* ms, any confiden */
	$"7469 616C 6974 7920 6F62 6C69 6761 7469"            /* tiality obligati */
	$"6F6E 7320 6F72 2075 7365 2072 6573 7472"            /* ons or use restr */
	$"6963 7469 6F6E 732C 2065 7870 7265 7373"            /* ictions, express */
	$"206F 7220 696D 706C 6965 642C 2077 6974"            /*  or implied, wit */
	$"6820 7265 7370 6563 7420 746F 2061 6E79"            /* h respect to any */
	$"2069 6E66 6F72 6D61 7469 6F6E 2074 6861"            /*  information tha */
	$"7420 596F 7520 6D61 7920 7072 6F76 6964"            /* t You may provid */
	$"6520 696E 2063 6F6E 6E65 6374 696F 6E20"            /* e in connection  */
	$"7769 7468 2074 6869 7320 4167 7265 656D"            /* with this Agreem */
	$"656E 7420 6F72 2074 6865 2050 726F 6772"            /* ent or the Progr */
	$"616D 2C20 696E 636C 7564 696E 67CA 0A69"            /* am, includingÊÂi */
	$"6E66 6F72 6D61 7469 6F6E 2061 626F 7574"            /* nformation about */
	$"2059 6F75 7220 4170 706C 6963 6174 696F"            /*  Your Applicatio */
	$"6E2C 204C 6963 656E 7365 6420 4170 706C"            /* n, Licensed Appl */
	$"6963 6174 696F 6E20 496E 666F 726D 6174"            /* ication Informat */
	$"696F 6E20 616E 6420 6D65 7461 6461 7461"            /* ion and metadata */
	$"2028 7375 6368 2064 6973 636C 6F73 7572"            /*  (such disclosur */
	$"6573 2077 696C 6C20 6265 2072 6566 6572"            /* es will be refer */
	$"7265 6420 746F 2061 7320 224C 6963 656E"            /* red to as "Licen */
	$"7365 6520 4469 7363 6C6F 7375 7265 7322"            /* see Disclosures" */
	$"292E 2059 6F75 2061 6772 6565 2074 6861"            /* ). You agree tha */
	$"7420 616E 7920 7375 6368 204C 6963 656E"            /* t any such Licen */
	$"7365 6520 4469 7363 6C6F 7375 7265 7320"            /* see Disclosures  */
	$"7769 6C6C 2062 6520 6E6F 6E2D 636F 6E66"            /* will be non-conf */
	$"6964 656E 7469 616C 2E20 414E 5343 4120"            /* idential. ANSCA  */
	$"7769 6C6C 2062 6520 6672 6565 2074 6F20"            /* will be free to  */
	$"7573 6520 616E 6420 6469 7363 6C6F 7365"            /* use and disclose */
	$"2061 6E79 204C 6963 656E 7365 6520 4469"            /*  any Licensee Di */
	$"7363 6C6F 7375 7265 7320 6F6E 2061 6E20"            /* sclosures on an  */
	$"756E 7265 7374 7269 6374 6564 2062 6173"            /* unrestricted bas */
	$"6973 2077 6974 686F 7574 206E 6F74 6966"            /* is without notif */
	$"7969 6E67 206F 7220 636F 6D70 656E 7361"            /* ying or compensa */
	$"7469 6E67 2059 6F75 2E20 596F 7520 7265"            /* ting You. You re */
	$"6C65 6173 6520 414E 5343 4120 6672 6F6D"            /* lease ANSCA from */
	$"2061 6C6C 206C 6961 6269 6C69 7479 2061"            /*  all liability a */
	$"6E64 206F 626C 6967 6174 696F 6E73 2074"            /* nd obligations t */
	$"6861 7420 6D61 7920 6172 6973 6520 6672"            /* hat may arise fr */
	$"6F6D 2074 6865 2072 6563 6569 7074 2C20"            /* om the receipt,  */
	$"7265 7669 6577 2C20 7573 652C 206F 7220"            /* review, use, or  */
	$"6469 7363 6C6F 7375 7265 206F 6620 616E"            /* disclosure of an */
	$"7920 706F 7274 696F 6E20 6F66 2061 6E79"            /* y portion of any */
	$"204C 6963 656E 7365 6520 4469 7363 6C6F"            /*  Licensee Disclo */
	$"7375 7265 732E 2041 6E79 2070 6879 7369"            /* sures. Any physi */
	$"6361 6C20 6D61 7465 7269 616C 7320 596F"            /* cal materials Yo */
	$"7520 7375 626D 6974 2074 6F20 414E 5343"            /* u submit to ANSC */
	$"4120 7769 6C6C 2062 6563 6F6D 6520 414E"            /* A will become AN */
	$"5343 4120 7072 6F70 6572 7479 2061 6E64"            /* SCA property and */
	$"2041 4E53 4341 2077 696C 6C20 6861 7665"            /*  ANSCA will have */
	$"206E 6F20 6F62 6C69 6761 7469 6F6E 2074"            /*  no obligation t */
	$"6F20 7265 7475 726E 2074 686F 7365 206D"            /* o return those m */
	$"6174 6572 6961 6C73 2074 6F20 596F 7520"            /* aterials to You  */
	$"6F72 2074 6F20 6365 7274 6966 7920 7468"            /* or to certify th */
	$"6569 7220 6465 7374 7275 6374 696F 6E2E"            /* eir destruction. */
	$"CA0A 0A43 2E20 5072 6573 7320 5265 6C65"            /* ÊÂÂC. Press Rele */
	$"6173 6573 2061 6E64 204F 7468 6572 2050"            /* ases and Other P */
	$"7562 6C69 6369 7479 2E20 596F 7520 6D61"            /* ublicity. You ma */
	$"7920 6E6F 7420 6973 7375 6520 616E 7920"            /* y not issue any  */
	$"7072 6573 7320 7265 6C65 6173 6573 206F"            /* press releases o */
	$"7220 6D61 6B65 2061 6E79 206F 7468 6572"            /* r make any other */
	$"2070 7562 6C69 6320 7374 6174 656D 656E"            /*  public statemen */
	$"7473 2072 6567 6172 6469 6E67 2074 6869"            /* ts regarding thi */
	$"7320 4167 7265 656D 656E 742C 2069 7473"            /* s Agreement, its */
	$"2074 6572 6D73 2061 6E64 2063 6F6E 6469"            /*  terms and condi */
	$"7469 6F6E 732C 206F 7220 7468 6520 7265"            /* tions, or the re */
	$"6C61 7469 6F6E 7368 6970 2077 6974 686F"            /* lationship witho */
	$"7574 2065 7870 7265 7373 2070 7269 6F72"            /* ut express prior */
	$"2077 7269 7474 656E 2061 7070 726F 7661"            /*  written approva */
	$"6C2C 2077 6869 6368 206D 6179 2062 6520"            /* l, which may be  */
	$"7769 7468 6865 6C64 2061 7420 414E 5343"            /* withheld at ANSC */
	$"4127 7320 6469 7363 7265 7469 6F6E 2ECA"            /* A's discretion.Ê */
	$"0A0A 0A36 2E20 5761 7272 616E 7479 2044"            /* ÂÂÂ6. Warranty D */
	$"6973 636C 6169 6D65 722E 2049 5420 4953"            /* isclaimer. IT IS */
	$"2055 4E44 4552 5354 4F4F 4420 5448 4154"            /*  UNDERSTOOD THAT */
	$"2054 4845 2044 4556 454C 4F50 4552 2053"            /*  THE DEVELOPER S */
	$"4F46 5457 4152 4520 414E 4420 414E 5920"            /* OFTWARE AND ANY  */
	$"5550 4441 5445 5320 4D41 5920 434F 4E54"            /* UPDATES MAY CONT */
	$"4149 4E20 4552 524F 5253 2E20 5448 4520"            /* AIN ERRORS. THE  */
	$"4445 5645 4C4F 5045 5220 534F 4654 5741"            /* DEVELOPER SOFTWA */
	$"5245 2041 4E44 2041 4E59 2055 5044 4154"            /* RE AND ANY UPDAT */
	$"4553 2041 5245 2050 524F 5649 4445 4420"            /* ES ARE PROVIDED  */
	$"2241 5320 4953 2220 5749 5448 4F55 5420"            /* "AS IS" WITHOUT  */
	$"5741 5252 414E 5459 204F 4620 414E 5920"            /* WARRANTY OF ANY  */
	$"4B49 4E44 2C20 5748 4554 4845 5220 4558"            /* KIND, WHETHER EX */
	$"5052 4553 532C 2049 4D50 4C49 4544 2C20"            /* PRESS, IMPLIED,  */
	$"5354 4154 5554 4F52 592C 204F 5220 4F54"            /* STATUTORY, OR OT */
	$"4845 5257 4953 452E 2041 4E53 4341 2041"            /* HERWISE. ANSCA A */
	$"4E44 2049 5453 2053 5550 504C 4945 5253"            /* ND ITS SUPPLIERS */
	$"2053 5045 4349 4649 4341 4C4C 5920 4449"            /*  SPECIFICALLY DI */
	$"5343 4C41 494D 2041 4C4C 2049 4D50 4C49"            /* SCLAIM ALL IMPLI */
	$"4544 2057 4152 5241 4E54 4945 5320 4F46"            /* ED WARRANTIES OF */
	$"204D 4552 4348 414E 5441 4249 4C49 5459"            /*  MERCHANTABILITY */
	$"2C20 4E4F 4E49 4E46 5249 4E47 454D 454E"            /* , NONINFRINGEMEN */
	$"542C 2041 4E44 2046 4954 4E45 5353 2046"            /* T, AND FITNESS F */
	$"4F52 2041 2050 4152 5449 4355 4C41 5220"            /* OR A PARTICULAR  */
	$"5055 5250 4F53 452E 2059 6F75 2061 636B"            /* PURPOSE. You ack */
	$"6E6F 776C 6564 6765 2074 6861 7420 414E"            /* nowledge that AN */
	$"5343 4120 6861 7320 6E6F 7420 7075 626C"            /* SCA has not publ */
	$"6963 6C79 2061 6E6E 6F75 6E63 6564 2074"            /* icly announced t */
	$"6865 2061 7661 696C 6162 696C 6974 7920"            /* he availability  */
	$"6F66 2074 6865 2044 6576 656C 6F70 6572"            /* of the Developer */
	$"2053 6F66 7477 6172 652C 2074 6861 7420"            /*  Software, that  */
	$"414E 5343 4120 6861 7320 6E6F 7420 7072"            /* ANSCA has not pr */
	$"6F6D 6973 6564 206F 7220 6775 6172 616E"            /* omised or guaran */
	$"7465 6564 2074 6F20 596F 7520 7468 6174"            /* teed to You that */
	$"2073 7563 6820 4465 7665 6C6F 7065 7220"            /*  such Developer  */
	$"536F 6674 7761 7265 2077 696C 6C20 6265"            /* Software will be */
	$"2061 6E6E 6F75 6E63 6564 206F 7220 6D61"            /*  announced or ma */
	$"6465 2061 7661 696C 6162 6C65 2074 6F20"            /* de available to  */
	$"616E 796F 6E65 2069 6E20 7468 6520 6675"            /* anyone in the fu */
	$"7475 7265 2C20 7468 6174 2041 4E53 4341"            /* ture, that ANSCA */
	$"2068 6173 206E 6F20 6578 7072 6573 7320"            /*  has no express  */
	$"6F72 2069 6D70 6C69 6564 206F 626C 6967"            /* or implied oblig */
	$"6174 696F 6E20 746F 2059 6F75 2074 6F20"            /* ation to You to  */
	$"616E 6E6F 756E 6365 206F 7220 696E 7472"            /* announce or intr */
	$"6F64 7563 6520 7468 6520 4465 7665 6C6F"            /* oduce the Develo */
	$"7065 7220 536F 6674 7761 7265 2C20 616E"            /* per Software, an */
	$"6420 7468 6174 2041 4E53 4341 206D 6179"            /* d that ANSCA may */
	$"206E 6F74 2069 6E74 726F 6475 6365 2061"            /*  not introduce a */
	$"2070 726F 6475 6374 2073 696D 696C 6172"            /*  product similar */
	$"206F 7220 636F 6D70 6174 6962 6C65 2077"            /*  or compatible w */
	$"6974 6820 7468 6520 4465 7665 6C6F 7065"            /* ith the Develope */
	$"7220 536F 6674 7761 7265 2E20 4163 636F"            /* r Software. Acco */
	$"7264 696E 676C 792C 2059 6F75 2061 636B"            /* rdingly, You ack */
	$"6E6F 776C 6564 6765 2074 6861 7420 616E"            /* nowledge that an */
	$"7920 7265 7365 6172 6368 206F 7220 6465"            /* y research or de */
	$"7665 6C6F 706D 656E 7420 7468 6174 2069"            /* velopment that i */
	$"7420 7065 7266 6F72 6D73 2072 6567 6172"            /* t performs regar */
	$"6469 6E67 2074 6865 2044 6576 656C 6F70"            /* ding the Develop */
	$"6572 2053 6F66 7477 6172 6520 6F72 2061"            /* er Software or a */
	$"6E79 2070 726F 6475 6374 2061 7373 6F63"            /* ny product assoc */
	$"6961 7465 6420 7769 7468 2074 6865 2044"            /* iated with the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6973 2064 6F6E 6520 656E 7469 7265"            /* e is done entire */
	$"6C79 2061 7420 596F 7572 206F 776E 2072"            /* ly at Your own r */
	$"6973 6B2E 2053 7065 6369 6669 6361 6C6C"            /* isk. Specificall */
	$"792C 2074 6865 2044 6576 656C 6F70 6572"            /* y, the Developer */
	$"2053 6F66 7477 6172 6520 6D61 7920 636F"            /*  Software may co */
	$"6E74 6169 6E20 6665 6174 7572 6573 2C20"            /* ntain features,  */
	$"6675 6E63 7469 6F6E 616C 6974 7920 6F72"            /* functionality or */
	$"206D 6F64 756C 6573 2074 6861 7420 7769"            /*  modules that wi */
	$"6C6C 206E 6F74 2062 6520 696E 636C 7564"            /* ll not be includ */
	$"6564 2069 6E20 7468 6520 7072 6F64 7563"            /* ed in the produc */
	$"7469 6F6E 2076 6572 7369 6F6E 206F 6620"            /* tion version of  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2C20 6966 2072 656C 6561"            /* ftware, if relea */
	$"7365 642C 206F 7220 7468 6174 2077 696C"            /* sed, or that wil */
	$"6C20 6265 206D 6172 6B65 7465 6420 7365"            /* l be marketed se */
	$"7061 7261 7465 6C79 2066 6F72 2061 6464"            /* parately for add */
	$"6974 696F 6E61 6C20 6665 6573 2E0A 0A0A"            /* itional fees.ÂÂÂ */
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
	$"6C6C 6172 7320 2824 3530 2E30 3029 2ECA"            /* llars ($50.00).Ê */
	$"2054 6865 2066 6F72 6567 6F69 6E67 206C"            /*  The foregoing l */
	$"696D 6974 6174 696F 6E73 2077 696C 6C20"            /* imitations will  */
	$"6170 706C 7920 6576 656E 2069 6620 7468"            /* apply even if th */
	$"6520 6162 6F76 6520 7374 6174 6564 2072"            /* e above stated r */
	$"656D 6564 7920 6661 696C 7320 6F66 2069"            /* emedy fails of i */
	$"7473 2065 7373 656E 7469 616C 2070 7572"            /* ts essential pur */
	$"706F 7365 2E0A 0A0A 382E 2045 7870 6F72"            /* pose.ÂÂÂ8. Expor */
	$"7420 436F 6E74 726F 6C2E 2059 6F75 206D"            /* t Control. You m */
	$"6179 206E 6F74 2075 7365 206F 7220 6F74"            /* ay not use or ot */
	$"6865 7277 6973 6520 6578 706F 7274 206F"            /* herwise export o */
	$"7220 7265 6578 706F 7274 2074 6865 2044"            /* r reexport the D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 6578 6365 7074 2061 7320 6175 7468"            /* e except as auth */
	$"6F72 697A 6564 2062 7920 556E 6974 6564"            /* orized by United */
	$"2053 7461 7465 7320 6C61 7720 616E 6420"            /*  States law and  */
	$"7468 6520 6C61 7773 206F 6620 7468 6520"            /* the laws of the  */
	$"6A75 7269 7364 6963 7469 6F6E 2873 2920"            /* jurisdiction(s)  */
	$"696E 2077 6869 6368 2074 6865 2044 6576"            /* in which the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"7761 7320 6F62 7461 696E 6564 2E20 496E"            /* was obtained. In */
	$"2070 6172 7469 6375 6C61 722C 2062 7574"            /*  particular, but */
	$"2077 6974 686F 7574 206C 696D 6974 6174"            /*  without limitat */
	$"696F 6E2C 2074 6865 2044 6576 656C 6F70"            /* ion, the Develop */
	$"6572 2053 6F66 7477 6172 6520 6D61 7920"            /* er Software may  */
	$"6E6F 7420 6265 2065 7870 6F72 7465 6420"            /* not be exported  */
	$"6F72 2072 652D 6578 706F 7274 6564 2028"            /* or re-exported ( */
	$"6129 2069 6E74 6F20 616E 7920 552E 532E"            /* a) into any U.S. */
	$"2065 6D62 6172 676F 6564 2063 6F75 6E74"            /*  embargoed count */
	$"7269 6573 206F 7220 2862 2920 746F 2061"            /* ries or (b) to a */
	$"6E79 6F6E 6520 6F6E 2074 6865 2055 2E53"            /* nyone on the U.S */
	$"2E20 5472 6561 7375 7279 2044 6570 6172"            /* . Treasury Depar */
	$"746D 656E 7427 7320 6C69 7374 206F 6620"            /* tment's list of  */
	$"5370 6563 6961 6C6C 7920 4465 7369 676E"            /* Specially Design */
	$"6174 6564 204E 6174 696F 6E61 6C73 206F"            /* ated Nationals o */
	$"7220 7468 6520 552E 532E 2044 6570 6172"            /* r the U.S. Depar */
	$"746D 656E 7420 6F66 2043 6F6D 6D65 7263"            /* tment of Commerc */
	$"6520 4465 6E69 6564 2050 6572 736F 6E27"            /* e Denied Person' */
	$"7320 4C69 7374 206F 7220 456E 7469 7479"            /* s List or Entity */
	$"204C 6973 742E 2042 7920 7573 696E 6720"            /*  List. By using  */
	$"7468 6520 4465 7665 6C6F 7065 7220 536F"            /* the Developer So */
	$"6674 7761 7265 2C20 596F 7520 7265 7072"            /* ftware, You repr */
	$"6573 656E 7420 616E 6420 7761 7272 616E"            /* esent and warran */
	$"7420 7468 6174 2059 6F75 2061 7265 206E"            /* t that You are n */
	$"6F74 206C 6F63 6174 6564 2069 6E20 616E"            /* ot located in an */
	$"7920 7375 6368 2063 6F75 6E74 7279 206F"            /* y such country o */
	$"7220 6F6E 2061 6E79 2073 7563 6820 6C69"            /* r on any such li */
	$"7374 2E20 596F 7520 616C 736F 2061 6772"            /* st. You also agr */
	$"6565 2074 6861 7420 596F 7520 7769 6C6C"            /* ee that You will */
	$"206E 6F74 2075 7365 2074 6865 2044 6576"            /*  not use the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"666F 7220 616E 7920 7075 7270 6F73 6573"            /* for any purposes */
	$"2070 726F 6869 6269 7465 6420 6279 2055"            /*  prohibited by U */
	$"6E69 7465 6420 5374 6174 6573 206C 6177"            /* nited States law */
	$"2C20 696E 636C 7564 696E 672C 2077 6974"            /* , including, wit */
	$"686F 7574 206C 696D 6974 6174 696F 6E2C"            /* hout limitation, */
	$"2074 6865 2064 6576 656C 6F70 6D65 6E74"            /*  the development */
	$"2C20 6465 7369 676E 2C20 6D61 6E75 6661"            /* , design, manufa */
	$"6374 7572 6520 6F72 2070 726F 6475 6374"            /* cture or product */
	$"696F 6E20 6F66 206E 7563 6C65 6172 2C20"            /* ion of nuclear,  */
	$"6368 656D 6963 616C 206F 7220 6269 6F6C"            /* chemical or biol */
	$"6F67 6963 616C 2077 6561 706F 6E73 2E0A"            /* ogical weapons.Â */
	$"0A0A 392E 2047 6F76 6572 6E6D 656E 7420"            /* ÂÂ9. Government  */
	$"456E 6420 5573 6572 732E 2054 6865 2044"            /* End Users. The D */
	$"6576 656C 6F70 6572 2053 6F66 7477 6172"            /* eveloper Softwar */
	$"6520 616E 6420 7265 6C61 7465 6420 646F"            /* e and related do */
	$"6375 6D65 6E74 6174 696F 6E20 6172 6520"            /* cumentation are  */
	$"2243 6F6D 6D65 7263 6961 6C20 4974 656D"            /* "Commercial Item */
	$"7322 2C20 6173 2074 6861 7420 7465 726D"            /* s", as that term */
	$"2069 7320 6465 6669 6E65 6420 6174 2034"            /*  is defined at 4 */
	$"3820 432E 462E 522E 20A4 322E 3130 312C"            /* 8 C.F.R. ¤2.101, */
	$"2063 6F6E 7369 7374 696E 6720 6F66 2022"            /*  consisting of " */
	$"436F 6D6D 6572 6369 616C 2043 6F6D 7075"            /* Commercial Compu */
	$"7465 7220 536F 6674 7761 7265 2220 616E"            /* ter Software" an */
	$"6420 2243 6F6D 6D65 7263 6961 6C20 436F"            /* d "Commercial Co */
	$"6D70 7574 6572 2053 6F66 7477 6172 6520"            /* mputer Software  */
	$"446F 6375 6D65 6E74 6174 696F 6E22 2C20"            /* Documentation",  */
	$"6173 2073 7563 6820 7465 726D 7320 6172"            /* as such terms ar */
	$"6520 7573 6564 2069 6E20 3438 2043 2E46"            /* e used in 48 C.F */
	$"2E52 2E20 A431 322E 3231 3220 6F72 2034"            /* .R. ¤12.212 or 4 */
	$"3820 432E 462E 522E 20A4 3232 372E 3732"            /* 8 C.F.R. ¤227.72 */
	$"3032 2C20 6173 2061 7070 6C69 6361 626C"            /* 02, as applicabl */
	$"652E 2043 6F6E 7369 7374 656E 7420 7769"            /* e. Consistent wi */
	$"7468 2034 3820 432E 462E 522E 20A4 3132"            /* th 48 C.F.R. ¤12 */
	$"2E32 3132 206F 7220 3438 2043 2E46 2E52"            /* .212 or 48 C.F.R */
	$"2E20 A432 3237 2E37 3230 322D 3120 7468"            /* . ¤227.7202-1 th */
	$"726F 7567 6820 3232 372E 3732 3032 2D34"            /* rough 227.7202-4 */
	$"2C20 6173 2061 7070 6C69 6361 626C 652C"            /* , as applicable, */
	$"2074 6865 2043 6F6D 6D65 7263 6961 6C20"            /*  the Commercial  */
	$"436F 6D70 7574 6572 2053 6F66 7477 6172"            /* Computer Softwar */
	$"6520 616E 6420 436F 6D6D 6572 6369 616C"            /* e and Commercial */
	$"2043 6F6D 7075 7465 7220 536F 6674 7761"            /*  Computer Softwa */
	$"7265 2044 6F63 756D 656E 7461 7469 6F6E"            /* re Documentation */
	$"2061 7265 2062 6569 6E67 206C 6963 656E"            /*  are being licen */
	$"7365 6420 746F 2055 2E53 2E20 476F 7665"            /* sed to U.S. Gove */
	$"726E 6D65 6E74 2065 6E64 2075 7365 7273"            /* rnment end users */
	$"2028 6129 206F 6E6C 7920 6173 2043 6F6D"            /*  (a) only as Com */
	$"6D65 7263 6961 6C20 4974 656D 7320 616E"            /* mercial Items an */
	$"6420 2862 2920 7769 7468 206F 6E6C 7920"            /* d (b) with only  */
	$"7468 6F73 6520 7269 6768 7473 2061 7320"            /* those rights as  */
	$"6172 6520 6772 616E 7465 6420 746F 2061"            /* are granted to a */
	$"6C6C 206F 7468 6572 2065 6E64 2075 7365"            /* ll other end use */
	$"7273 2070 7572 7375 616E 7420 746F 2074"            /* rs pursuant to t */
	$"6865 2074 6572 6D73 2061 6E64 2063 6F6E"            /* he terms and con */
	$"6469 7469 6F6E 7320 6865 7265 696E 2E20"            /* ditions herein.  */
	$"556E 7075 626C 6973 6865 642D 7269 6768"            /* Unpublished-righ */
	$"7473 2072 6573 6572 7665 6420 756E 6465"            /* ts reserved unde */
	$"7220 7468 6520 636F 7079 7269 6768 7420"            /* r the copyright  */
	$"6C61 7773 206F 6620 7468 6520 556E 6974"            /* laws of the Unit */
	$"6564 2053 7461 7465 732E 0A0A 0A31 302E"            /* ed States.ÂÂÂ10. */
	$"2043 6F6E 7472 6F6C 6C69 6E67 204C 6177"            /*  Controlling Law */
	$"2061 6E64 2053 6576 6572 6162 696C 6974"            /*  and Severabilit */
	$"792E 2054 6869 7320 4C69 6365 6E73 6520"            /* y. This License  */
	$"7769 6C6C 2062 6520 676F 7665 726E 6564"            /* will be governed */
	$"2062 7920 616E 6420 636F 6E73 7472 7565"            /*  by and construe */
	$"6420 696E 2061 6363 6F72 6461 6E63 6520"            /* d in accordance  */
	$"7769 7468 2074 6865 206C 6177 7320 6F66"            /* with the laws of */
	$"2074 6865 2053 7461 7465 206F 6620 4361"            /*  the State of Ca */
	$"6C69 666F 726E 6961 2C20 6173 2061 7070"            /* lifornia, as app */
	$"6C69 6564 2074 6F20 6167 7265 656D 656E"            /* lied to agreemen */
	$"7473 2065 6E74 6572 6564 2069 6E74 6F20"            /* ts entered into  */
	$"616E 6420 746F 2062 6520 7065 7266 6F72"            /* and to be perfor */
	$"6D65 6420 656E 7469 7265 6C79 2077 6974"            /* med entirely wit */
	$"6869 6E20 4361 6C69 666F 726E 6961 2062"            /* hin California b */
	$"6574 7765 656E 2043 616C 6966 6F72 6E69"            /* etween Californi */
	$"6120 7265 7369 6465 6E74 732E 2054 6869"            /* a residents. Thi */
	$"7320 4C69 6365 6E73 6520 7368 616C 6C20"            /* s License shall  */
	$"6E6F 7420 6265 2067 6F76 6572 6E65 6420"            /* not be governed  */
	$"6279 2074 6865 2055 6E69 7465 6420 4E61"            /* by the United Na */
	$"7469 6F6E 7320 436F 6E76 656E 7469 6F6E"            /* tions Convention */
	$"206F 6E20 436F 6E74 7261 6374 7320 666F"            /*  on Contracts fo */
	$"7220 7468 6520 496E 7465 726E 6174 696F"            /* r the Internatio */
	$"6E61 6C20 5361 6C65 206F 6620 476F 6F64"            /* nal Sale of Good */
	$"732C 2074 6865 2061 7070 6C69 6361 7469"            /* s, the applicati */
	$"6F6E 206F 6620 7768 6963 6820 6973 2065"            /* on of which is e */
	$"7870 7265 7373 6C79 2065 7863 6C75 6465"            /* xpressly exclude */
	$"642E 2049 6620 666F 7220 616E 7920 7265"            /* d. If for any re */
	$"6173 6F6E 2061 2063 6F75 7274 206F 6620"            /* ason a court of  */
	$"636F 6D70 6574 656E 7420 6A75 7269 7364"            /* competent jurisd */
	$"6963 7469 6F6E 2066 696E 6473 2061 6E79"            /* iction finds any */
	$"2070 726F 7669 7369 6F6E 2C20 6F72 2070"            /*  provision, or p */
	$"6F72 7469 6F6E 2074 6865 7265 6F66 2C20"            /* ortion thereof,  */
	$"746F 2062 6520 756E 656E 666F 7263 6561"            /* to be unenforcea */
	$"626C 652C 2074 6865 2072 656D 6169 6E64"            /* ble, the remaind */
	$"6572 206F 6620 7468 6973 204C 6963 656E"            /* er of this Licen */
	$"7365 2073 6861 6C6C 2063 6F6E 7469 6E75"            /* se shall continu */
	$"6520 696E 2066 756C 6C20 666F 7263 6520"            /* e in full force  */
	$"616E 6420 6566 6665 6374 2E20 CA0A 0A0A"            /* and effect. ÊÂÂÂ */
	$"3131 2E20 436F 6D70 6C65 7465 2041 6772"            /* 11. Complete Agr */
	$"6565 6D65 6E74 3B20 476F 7665 726E 696E"            /* eement; Governin */
	$"6720 4C61 6E67 7561 6765 2E20 5468 6973"            /* g Language. This */
	$"204C 6963 656E 7365 2063 6F6E 7374 6974"            /*  License constit */
	$"7574 6573 2074 6865 2065 6E74 6972 6520"            /* utes the entire  */
	$"6167 7265 656D 656E 7420 6265 7477 6565"            /* agreement betwee */
	$"6E20 7468 6520 7061 7274 6965 7320 7769"            /* n the parties wi */
	$"7468 2072 6573 7065 6374 2074 6F20 7468"            /* th respect to th */
	$"6520 7573 6520 6F66 2074 6865 2044 6576"            /* e use of the Dev */
	$"656C 6F70 6572 2053 6F66 7477 6172 6520"            /* eloper Software  */
	$"6C69 6365 6E73 6564 2068 6572 6575 6E64"            /* licensed hereund */
	$"6572 2061 6E64 2073 7570 6572 7365 6465"            /* er and supersede */
	$"7320 616C 6C20 7072 696F 7220 6F72 2063"            /* s all prior or c */
	$"6F6E 7465 6D70 6F72 616E 656F 7573 2075"            /* ontemporaneous u */
	$"6E64 6572 7374 616E 6469 6E67 7320 7265"            /* nderstandings re */
	$"6761 7264 696E 6720 7375 6368 2073 7562"            /* garding such sub */
	$"6A65 6374 206D 6174 7465 722E 204E 6F20"            /* ject matter. No  */
	$"616D 656E 646D 656E 7420 746F 206F 7220"            /* amendment to or  */
	$"6D6F 6469 6669 6361 7469 6F6E 206F 6620"            /* modification of  */
	$"7468 6973 204C 6963 656E 7365 2077 696C"            /* this License wil */
	$"6C20 6265 2062 696E 6469 6E67 2075 6E6C"            /* l be binding unl */
	$"6573 7320 696E 2077 7269 7469 6E67 2061"            /* ess in writing a */
	$"6E64 2073 6967 6E65 6420 6279 2041 4E53"            /* nd signed by ANS */
	$"4341 2E20 416E 7920 7472 616E 736C 6174"            /* CA. Any translat */
	$"696F 6E20 6F66 2074 6869 7320 4167 7265"            /* ion of this Agre */
	$"656D 656E 7420 6973 2064 6F6E 6520 666F"            /* ement is done fo */
	$"7220 6C6F 6361 6C20 7265 7175 6972 656D"            /* r local requirem */
	$"656E 7473 2061 6E64 2069 6E20 7468 6520"            /* ents and in the  */
	$"6576 656E 7420 6F66 2061 2064 6973 7075"            /* event of a dispu */
	$"7465 2062 6574 7765 656E 2074 6865 2045"            /* te between the E */
	$"6E67 6C69 7368 2061 6E64 2061 6E79 206E"            /* nglish and any n */
	$"6F6E 2D45 6E67 6C69 7368 2076 6572 7369"            /* on-English versi */
	$"6F6E 2C20 7468 6520 456E 676C 6973 6820"            /* on, the English  */
	$"7665 7273 696F 6E20 6F66 2074 6869 7320"            /* version of this  */
	$"4167 7265 656D 656E 7420 7368 616C 6C20"            /* Agreement shall  */
	$"676F 7665 726E 2E0A 0A0A 3132 2E20 5468"            /* govern.ÂÂÂ12. Th */
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
	$"0A0A 3230 3130 2D30 392D 3039 0A0A 00"              /* ÂÂ2010-09-09ÂÂ. */
};

