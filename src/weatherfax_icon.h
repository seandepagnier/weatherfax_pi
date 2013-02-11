/*  GIMP header image file format (RGB): /home/sean/build/OpenCPN/plugins/weatherfax_pi/src/fax.h  */

static unsigned int width = 32;
static unsigned int height = 32;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}
static const char *header_data =
	"`P\\_`P\\_`P\\_````````````````````````````````````````````````````"
	"`````````````````````````````````````````````````````P\\_`P\\_````"
	"`P\\_`P\\_`P\\_````````````````````````````````````````````````````"
	"`````````````````````````````````````````````````````P\\_`P\\_````"
	"`````````````P\\_`@X^`0T]_`P\\_`P\\_`P\\_`P\\_PP[_@PX_@LX_@LX_@LX_@LX"
	"_@LX_@LX_@LX_@PX_@PX_PPZ_`P\\_`P\\_`P\\_PL[_`P\\`0T]`P\\_`````````P\\_"
	"````````````]`0TY/$AT-T-PL[_P,S]P<S_PLT!K\\C>EL*OC[ZEC;RFC;RFCKRG"
	"CKRGC;RFC;RFC[ZFEL&NJ,#7M;_TM+_RLK[OL;WNP<W^VN<7\\_`P`````````P\\_"
	"````````````Z?8FR-4%O,CYM\\/TML+SL[WQK+;MBJFR8IUI;*9M?[*!?[*!?;%`"
	"?;%`?[*!?[*!;:=N8IUIA**LGZC?G:C<G*C9GJK;I;'BML+SX>X>````````````"
	"````````````VN<7LK[OR]@(XN\\?W^P<UN(4QM$&F+;\";*1WG<F=T^_/T^_/S^W,"
	"S^W,T^_/T^_/G<F=;*1WFKC$P\\T#O\\K^PL[_RM<'L[_PFJ;7T-T-````````````"
	"`````````0T]U>(2K[OLS]P,YO,CT]`0S]L-VN09J,?2;:1XH<R@W??8W??8V?34"
	"V?34W??8W??8H<R@;:1XI,+.TMT2R-0'Q]0$T=X.M\\/TF:76S-D)`P\\_````````"
	"`````P\\_^`@XT]`0L;WNT-T-XN\\?P\\_`QM$$Z/(GM=3@;:5XG<B<V/+3V/+3U/#0"
	"U/#0V/+3V/+3GLF=;:1XJ<C3VN4:S]H-R]@(T=X.N<7VH*S=S-D)_0DY````````"
	"`````0T]]P,ST=X.L[_PT=X.XN\\?QM,#RM8)[/<LN-;B;:5XG<B<V//3V//3U/#0"
	"U/#0V//3V//3GLF>;:1XJ,;2W.8<V^89U^04T]`0N\\?XI;'BR]@(]@(R`0T]````"
	"````_PL[\\/TMSML+M,#QS]P,Y/$AVN49V.$:W>4@JL;5;:1XG<F<UO+2U_+2T^_."
	"T^_.U_+2UO+2G<F=;:1XJ,33VN(=UM\\8T]X2T]`0OLKZJ+3ER=8&\\/TM_PL[````"
	"````_0DYZO<GR]@(M,#QSMH+Y/$AXO$>U>(1O,?ZD*^Y;*1WGLJ=U/#.U/#.T.W+"
	"T.W+U/#.U/#.G<F<;*1WG[[(R-4&O<KYO\\W[T-T,P,S]JK;GQM,#ZO<G_0DY````"
	"````^P<WY/$AQ]0$M<'RT]X3V>D1G\\6]@+\"3@*R8<**$9J!RG,B=T.W)T.W)S.K&"
	"S.K&T.W)T.W)G,>=9J!R=:>)A+\"<>:F-C[:NQ-3]P\\T#K;GJQ-$!Y/$A^P<W````"
	"````]`0TW>H:Q=(\"ML+SR=,)Q=7]C+6G;*1U9Z)M;J=S@;2'JM&HS.G#S.G#R>?!"
	"R>?!S.G#S.G#J]&I@K2';:9R9J%L;Z=YBK.EL\\/KN,+YK[OLP\\_`WNL;^04U````"
	"````]@(RU^04Q-$!M\\/TK[KNK;KIL\\?FH,+#>*N&B;J*M-FOPN.YQ^6]Q^6]QN6\\"
	"QN6\\QN6\\Q^6]Q..ZM]JQB;F*=JN%H,/#K\\7BH:[<I*[BL+SMPL[_V.45]P,S````"
	"````\\_`PT=X.P<W^N<7VK[OML;WNR-8$PM?TG\\.^D+R?D<\"3J]*CP^*WQ..XP^.W"
	"PN.WQ.2XQ..XKM2FE<*6C;F;E[VVOM3PP]'_I[/DIK'CL[_PO\\O\\T=X.\\_`P````"
	"````\\/TMRM<'OLK[O,CYN,3UOLK[UN$5V><4Q-GVGL*`>:R*D+Z1L-6GN]VOP.&S"
	"P.&SO-ZPL]>ID\\\"2>JV(E[VWNM'IT=`,S]H.M,#QK[OLML+SN\\?XR]@(\\?XN````"
	"````[OLKQ-$!NL;WOLK[PL[_RM<(VN<8W^P=V.85P-;SH,/\"B+:9@[6%ILV<O]ZN"
	"P.\"PJ]&AB+B*A;23E;RRM\\_GTM\\.V>46TM\\/P,S]N,3UN,3UN,3UQM,#[_PL````"
	"````Z_@HO<GZML+SP<W^R=8&T]`0W^P;Y?$CYO$DV^H6Q]OYGL*_@;&1D+V1GL>6"
	"HLJ9E,\"3@+\"-E[ZVOM;MUN80X^XBXNX?V>86RM<'P,S]N\\?XM<'RO\\O\\[/DI````"
	"````Z?8FML+SL[_PP\\_`SML+U^04X.T=YO,CZ?8FZ_8IY.`BQ-OTH,2_AK66>JY`"
	"?;\"!AK62EKZRO=?LY/$BZ_<JZ?8FYO,CW>H:T-T-QM,#OLK[LK[OM\\/TZO<G````"
	"````Y_0DL;WNLK[OQ]0$T=X.U^04V.45VN<7WNL;Y.`AZ?4FZ/<DV.T+I<O\"@K2/"
	">*V!E\\*OTND$YO4BZ/0FX^`AW.D9U^04T]`0T=X.S-D)P\\_`L;WNL;WNY_0D````"
	"````YO,CK+CIJ[?HO<GZP,S]Q-$!S=H*T]`0U.$1V>86X.T=[/<LZ/8FP-GLHL7\""
	"EKVSLM#;YO0DZ_8LW^P<V.45TM\\/SML+R-4%P<W^O,CYN<7VJ;7FK+CIYO,C````"
	"````Y/$AI[/DH*S=JK;GHJ[?IK+CQ=(\"T]`0T=X.T=X.TM\\/TM\\/TM\\/T-X,S]T*"
	"SMP)S]T+T=X.T=X.T-T-S]P,SML+S]P,PL[_I[/DH:W>IK+CGJK;J+3EY?(B````"
	"````X^`@IK+CKKKKS-D)T=X.UN,3X^`@Z/4EYO,CY?(BY/$AXN\\?XN\\?Y.`CYO$E"
	"Y_(FY>`CX>T>X.T=X.T=X>X>XN\\?X^`@W>H:T-T-S=H*R]@(KKKKIK+CX^`@````"
	"````W.D9DI[/HJ[?S=H*SML+RM<'R]@(S-D)S]P,T=X.T]`0U.$1UN,3U^04U^04"
	"V.45V.45V.45V.45V.45U^04UN,3UN,3UN,3V.45UN,3S=H*HJ[?DI[/W.D9````"
	"````RM<'66686V>9>(2U35F*,3UN04U^45V.7VN<:W>H=8&R?XN\\AY/$CIK+DI[/"
	"E:'2EJ+3EJ+3E:'2DY_0CYO,BY?(AY/$A)#!@X^`@(R]>(2U6&2676F<R]@)````"
	"````Q]0%3EJ-0T^!6&24/$AX*C9G-T-T1%\"!3UN,66668FZ?;'BI=(\"Q>H:W?XN\\"
	"@8V^@HZ_@8V^?XN\\?(BX=H*S<'RM:W>H9W.D97&B7VN<5V.40T^!5&\"3R=8'````"
	"````SML,8V^B2E:(4EZ/66668&R=8FZ?9'\"A97&B:'2E;'BI<7VN=8&S>86V>X>X"
	">X>X>X>X>X>X>(2U=(\"Q;WNL:76F9'\"A86V>7VN<6&2545V.3UN-;7FKTM\\/````"
	"````V^@8A)#\"3EJ-+SMN,#QO-T-U-T-U-T-V-T-V.$1V.45W.D9X.T=Z/$AZ/$A["
	"/$A[/$A[/$A[.T=Z.D9Y.45W-T-V-D)T-4%S-$!S+CIL+SMM7&B:FJ;8X^`@````"
	"````[?HJN\\?YCIK,;'BK:G:H;7FK;'BK;'BK;'BK;'BJ;'BJ:W>J:W>I:W>I:G:I"
	":G:I:G:I:G:I:W>I:W>I:W>J;'BJ;'BK;'BK;'BK:76G;'BKFJ;8SML,]0$Q````"
	"`````@X^_`P\\_@HZ^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X"
	"^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X^`@X_@HZ`0T]`P\\_````"
	"`P\\_````````````````````````````````````````````````````````````"
	"`````````````````````````````````````````````````````````````P\\_"
	"````````````````````````````````````````````````````````````````"
	"````````````````````````````````````````````````````````````````"
	"";
