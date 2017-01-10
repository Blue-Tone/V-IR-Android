/*
  Digispark と HID で通信する
	http://www.microchip.com/forums/m340898.aspx
  http://libusb.sourceforge.net/api-1.0/api.html
  http://kernhack.hatenablog.com/entry/20100303/1267615931
  http://www.kumikomi.net/archives/2007/03/22usb1.php?page=1
  http://www.itf.co.jp/tech/road-to-usb-master  
 */

#include <errno.h> 
#include <string.h> 
#include <stdio.h> 
#include <stdlib.h> 

#include <unistd.h>
#include <sys/types.h>
#include "version.h"

// Linux ではディストリビューションによって必要な方をインクルードする
#if LINUX
#include <libusb-1.0/libusb.h> 
//#include <libusb/libusb.h>
#endif

#if FreeBSD
#include <libusb.h>  // FreeBSD ではこれだけをインクルードする
#endif

#if MINGW32
#include <libusb.h>  // MING ではこれだけをインクルードする
#endif

#include <unistd.h>

// 有効なベンダID, プロダクトID の対を列挙
uint16_t VENDOR_IDS[] =  { 0x16c0, 0x20a0, 0x20a0 }; // ベンダID: 縦方向にペアなので注意
uint16_t PRODUCT_IDS[] = { 0x05df, 0x0001, 0x427e }; // プロダクトID: 縦方向にペアなので注意

// HID Class-Specific Requests values(section 7.2 of the HID specifications) 
#define HID_GET_REPORT                0x01 
#define HID_GET_IDLE                  0x02 
#define HID_GET_PROTOCOL              0x03 
#define HID_SET_REPORT                0x09 
#define HID_SET_IDLE                  0x0A 
#define HID_SET_PROTOCOL              0x0B 
#define HID_REPORT_TYPE_INPUT         0x01 
#define HID_REPORT_TYPE_OUTPUT        0x02 
#define HID_REPORT_TYPE_FEATURE       0x03 
   
#define CTRL_IN  LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE 
#define CTRL_OUT LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE 
 
const static int PACKET_CTRL_LEN=8;
const static int PACKET_INT_LEN=2; 
const static int INTERFACE=0; 
const static int ENDPOINT_INT_IN=0x81; /* endpoint 0x81 address for IN */ 
const static int ENDPOINT_INT_OUT=0x01; /* endpoint 1 address for OUT */ 
const static int TIMEOUT=5000; /* timeout in ms */  

// デバイスハンドルの定義
static struct libusb_device_handle *devh = NULL;  

// geteuid
#ifdef MINGW32
int geteuid(void) { return 0; }
#endif


//char *libusb_error_text2(int errno);
	
// エラーメッセージ
char *libusb_error_text(int err_number) {
	switch (err_number) {
	case 0:
		return "LIBUSB_SUCCESS";
	case -1:
		return "LIBUSB_ERROR_IO";
	case -2:
		return "LIBUSB_ERROR_INVALID_PARAM";
	case -3:
		return "LIBUSB_ERROR_ACCESS";
	case -4:
		return "LIBUSB_ERROR_NO_DEVICE";
	case -5:
		return "LIBUSB_ERROR_NOT_FOUND";
	case -6:
		return "LIBUSB_ERROR_BUSY";
	case -7:
		return "LIBUSB_ERROR_TIMEOUT";
	case -8:
		return "LIBUSB_ERROR_OVERFLOW";
	case -9:
		return "LIBUSB_ERROR_PIPE";
	case -10:
		return "LIBUSB_ERROR_INTERRUPTED";
	case -11:
		return "LIBUSB_ERROR_NO_MEM";
	case -12:
		return "LIBUSB_ERROR_NOT_SUPPORTED";
	case -99:
		return "LIBUSB_ERROR_OTHER";
	}
	return "UNDEFINED ERROR";
}

// main
int main(int argc, char **argv) {
  int r = 1;  // return code
	int opt;
	int opt_raw = 0;
	int opt_debug = 0;
	float opt_freq = 38;
	char cmd;
	char *filename = NULL;
	
	while ((opt=getopt(argc, argv, "f:rv"))!=-1) {
		if (opt=='r') opt_raw   = 1;
		if (opt=='v') opt_debug = 1;
		if (opt=='f') sscanf(optarg,"%f", &opt_freq);
	}
	
	if (argc-1 < optind) goto usage;
	cmd = argv[optind][0];
	if (opt_debug) fprintf(stderr, "CMD:%c\n", cmd);
		
	if (cmd!='R' && cmd!='S' && cmd!='P' && cmd!='s' && cmd != 'l' && cmd !='V') {

	usage:
		fprintf(stderr, "USAGE:\n");
		fprintf(stderr, " V-IR [-r(raw)][-f freq][-v(verb)] R > FILE.ir\tRECEIVE IR DATA & WRITE TO RAM\n");
		fprintf(stderr, " V-IR S < FILE.ir\t\tTRANSMIT IR DATA & WRITE TO RAM\n");
		fprintf(stderr, " V-IR P\t\t\t\tSHOW IR DATA IN RAM\n");
		fprintf(stderr, " V-IR s\t\t\t\tSAVE DATA FROM RAM TO EEPROM\n");
		fprintf(stderr, " V-IR l\t\t\t\tLOAD DATA FROM EEPROM TO RAM & TRANSMIT IT\n");
		fprintf(stderr, " V-IR V\t\t\t\tSHOW INFO OF AVR-FIRMWARE\n");
		fprintf(stderr, "Please run this command as a root user.\n");
		printf("(VERSION=%s, CMD=%s, EUID=%d)\n", VERSION, argv[0], geteuid());
		exit(1);
	}
	// ファイル名の保存
	if (optind == argc-2) filename = argv[optind+1];

	// キャリア周波数の指定(R の場合のみ)
	if (cmd=='R') {
		if (opt_debug) fprintf(stderr, "Carrier frequency = %f kHz (N=%02X)\n",
													 opt_freq, (int)(0.5+16.5*1000*1000/(opt_freq*1000.0*2)));
		if (opt_debug) fprintf(stderr, "OPT-RAW:%d\n", opt_raw);
	}
	
	// USB の初期化
	r = libusb_init(NULL); 
	if (r < 0) { 
		fprintf(stderr, "Failed to initialise libusb\n"); 
		exit(1); 
	} 

	// HID デバイスの検索(1個だけ)
	int loop;
	for (loop=0; loop<sizeof(VENDOR_IDS)/sizeof(uint16_t); loop++) {
		devh = libusb_open_device_with_vid_pid(NULL, VENDOR_IDS[loop], PRODUCT_IDS[loop]);
		if (devh) break;
	}
	if (!devh) { 
		int loop;
		for (loop=0; loop<sizeof(VENDOR_IDS)/sizeof(uint16_t); loop++) {
			fprintf(stderr, "Searching a USB device (VENDOR_ID=%04XH, PRODUCT_ID=%04XH)... not found.\n",VENDOR_IDS[loop], PRODUCT_IDS[loop]);
		}
#ifndef MINGW32
		fprintf(stderr, "Please run this command as a root user.\n");
#endif
		// 失敗したらクローズして終了
		libusb_close(devh); 
		libusb_exit(NULL); 
		exit(1);
	} 
	if (opt_debug) fprintf(stderr,"Generic HID device found\n"); 
 
#ifdef Linux
	// OS デバイスドライバの無効化
	libusb_detach_kernel_driver(devh, 0);      
#endif 

	// USB デバイスの初期化(configration)
	r = libusb_set_configuration(devh, 1); 
	if (r < 0) { 
		fprintf(stderr, "libusb_set_configuration error: %s\n", libusb_error_text(r)); 
		libusb_close(devh); 
		libusb_exit(NULL); 
		exit(1);
	} 
	if (opt_debug) fprintf(stderr,"libusb_set_configuration OK\n"); 

	int i; 
	char answer[PACKET_CTRL_LEN]; 
	 char question[PACKET_CTRL_LEN];
	 char c;
	 unsigned char j=0;
	 unsigned int cnt=0;
	 int lfflag ;
	 FILE *fp;

	 ////////////////////////////////////////////////////////////
	 // コマンドで分岐
	 ////////////////////////////////////////////////////////////
	 switch (cmd) {
		 char *buf;      // USB からのデータの一時保存用
		 int read_size;	 // buf[] の現在のサイズ
		 // ------------------- S: 送信(標準入力をそのまま送信する)
	 case 'S':
		 lfflag = 0;
		 read_size=0;
		 buf=NULL;
		 unsigned int fncnt=1;
		 
		 // ファイルから生データを取得
		 do {
			 // fp の設定
			 if (optind+fncnt>=argc) fp=stdin;
			 else {
				 filename = argv[optind+fncnt];
				 if ((fp=fopen(filename, "r")) == NULL) {
					 fprintf(stderr, "Can't open '%s' for read\n", filename);
					 exit(1);
				 }
			 }
			 // fp からデータ読み出し
			 while (!feof(fp)) {
				 int c = fgetc(fp);
				 if (c==EOF) break;
				 if (c=='\n') lfflag=1;
				 buf = realloc(buf, read_size+1);
				 buf[read_size]=c;
				 read_size++;
			 }
			 fclose(fp);
			 // 次のファイル
			 fncnt++;
		 } while (optind+fncnt<argc);

		 // 改行がなかった
		 if (lfflag==0) { fprintf(stderr, "***** DATA ERROR: Data is not terminated with LN\n"); break; } 

		 // 複数デバイス対応(2016/10/11)
		 // 複数ファイル対応(2016/11/04)
		 libusb_close(devh);	// 通常の devh は close
		 {
			 libusb_device **list;
			 ssize_t cnt = libusb_get_device_list(NULL, &list); // cnt:USBデバイスの数, list[]: USBデバイス
			 ssize_t i = 0;
			 // int sentflag = 0;
			 // USB デバイスが見つからない
			 if (cnt < 0) {
				 fprintf(stderr, "libusb_get_device_list(): NO USB device found.\n");
				 libusb_exit(NULL); 
				 exit(1);
			 }
			 // 発見した USB デバイスの数だけループ
			 for (i=0; i<cnt; i++) {
				 libusb_device *device = list[i];
				 struct libusb_device_descriptor desc;
				 // list[] からディスクリプタを取得
				 int r = libusb_get_device_descriptor(device, &desc);
				 if (r < 0) {
					 fprintf(stderr, "libusb_get_device_descriptor error %s.\n", libusb_error_text(r));
					 libusb_exit(NULL); 
					 exit(1);
				 }
				 // ディスクリプタから VENDOR_ID, PRODUCT_ID を取得して比較
				 int idflag;
				 int loop;
				 idflag = 0;
				 for (loop=0; loop<sizeof(VENDOR_IDS)/sizeof(uint16_t); loop++) {
					 if (desc.idVendor==VENDOR_IDS[loop] && desc.idProduct==PRODUCT_IDS[loop]) idflag=1;
				 }
				 if (idflag) {
					 int p=0; // buf 内の位置
					 int transfer_err_cnt=0; // エラー回数
					 // sentflag=1; // 1度でも送信したら1
					 // buf の最後まで繰り返す(複数 IR データ連続送出に対応)
					 while (p<read_size) {
						 // 独自に devh を open
						 r = libusb_open(device, &devh);
						 if (r < 0) {
							 fprintf(stderr, "libusb_open error %s.\n", libusb_error_text(r));
							 libusb_exit(NULL); 
							 exit(1);
						 }
						 // インタフェースの要求
						 r = libusb_claim_interface(devh, 0); 
						 if (r < 0) { 
							 fprintf(stderr, "libusb_claim_interface error %s\n", libusb_error_text(r));
							 // 失敗したらクローズして終了
							 libusb_close(devh); 
							 libusb_exit(NULL); 
						 } 
						 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 
						 // コントロール転送(ホスト→デバイス)
						 while (p<read_size) {
							 question[0] = buf[p];
							 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																					 0, question, PACKET_CTRL_LEN,TIMEOUT);
							 if (r < 0) {
								 if (opt_debug) fprintf(stderr, "Control out: libusb_control_transfer error: %s\n", libusb_error_text(r));
								 usleep(300*1000);
								 transfer_err_cnt++;
								 break;
							 } // 戻り値が負ならエラー
							 p++;
						 } // while (p<read_size) 
						 // インタフェースの解放
						 libusb_release_interface(devh, 0);
						 // 独自に devh を close
						 libusb_close(devh);
						 if (transfer_err_cnt>10) {
							 // "Control out エラーによる無限ループ防止
							 fprintf(stderr, "Control out: Too many libusb_control_transfer error.\n");
							 break; 
						 }
					 }  // while (p<read_size) // buf の最後まで繰り返す
				 } // if VENDOR_ID, PRODUCT_ID
			 } // for i	(複数デバイスでループ)
			 // 既存の devh は close 済みなので多重 close を防ぐためにここで exit 
			 libusb_exit(NULL); 
			 exit(0);
		 } // 複数デバイス対応
		 break;
		 
		 // ------------------- R: 学習指示(コマンド(02H) と引数で指定した学習パラメータを送信する)+受信
	 case 'R':
		 // USB からの受信データは一旦 buf[] に格納して、成功後にファイルに書き出す
		 buf = NULL;

		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 break; // 失敗したらクローズして終了
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 

		 // 学習コマンドの送信
		 char R_CMD[9]  =  {'#',',','0','2',',','D','9','\r','\n'};
		 unsigned char str[3];
		 unsigned char N;		 

		 // 2,3 バイト目がコマンド
		 if (opt_raw) sprintf(str, "%02X", 0x02 | 0x10); else sprintf(str, "%02X", 0x02);
		 R_CMD[2] = str[0];
		 R_CMD[3] = str[1];

		 // 5,6 バイト目が分周比N（N = 16.5*1000*1000/freq)
		 N = (int)(0.5+16.5*1000*1000/(opt_freq*1000*2)); // 2倍サンプリングなので分周比は 1/2
		 sprintf(str, "%02X", N);
		 R_CMD[5] = str[0];
		 R_CMD[6] = str[1];
		 
		 for (j=0; j<9; j++) {
			 question[0] = R_CMD[j];
			 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, question, PACKET_CTRL_LEN,TIMEOUT);
			 if (r < 0) { fprintf(stderr, "Control OUT: libusb_control_transfer error: %s\n",
														libusb_error_text(r)); goto outerror; } // 戻り値が負ならエラー
		 }

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);

		 
		 // インタフェースの要求
		 fprintf(stderr,"Waiting...\n");

	 redo_R:
		 read_size=0;
		 buf=NULL;
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 // 失敗したらクローズして終了
			 //libusb_close(devh); 
			 //libusb_exit(NULL);
			 //exit(1);
			 goto redo_R;
		 } 
		 // fprintf(stderr,"Successfully claimed interface\n"); 

		 // コントロール転送(デバイス→ホスト)
		 do {
			 r = libusb_control_transfer(devh,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, answer,PACKET_CTRL_LEN, TIMEOUT); 
			 if (r < 0) {
				 libusb_release_interface(devh, 0);
				 // fprintf(stderr, "Control IN error %d\n", r);
				 goto redo_R;
			 } // 戻り値が負ならインタフェースを解放して再取得から再開
			 c = answer[0];
			 if (r>0) {
				 if (c=='\r') continue; // \r だけは書かない
				 buf = realloc(buf, read_size+1);
				 buf[read_size]=c;
				 read_size++;
				 // fprintf(fp, "%c", c);
			 }
		 } while (c != '\n');

		 
		 // インタフェースの解放
		 libusb_release_interface(devh, 0);

		 if (read_size>3) {
			 int i;
			 // 学習に成功した場合はファイルに書き出す
			 // fp はファイルか標準入力
			 if (filename==NULL) {
				 fp=stdout;
			 } else {
				 if ((fp=fopen(filename, "w")) == NULL) {
					 fprintf(stderr, "Can't open '%s' for write.\n", filename);
					 exit(1);
				 } // if ((fp=fopen(filename, "w")) == NULL)
			 }	// if (filename==NULL)
			 for (i=0; i<read_size; i++) {
				 fprintf(fp, "%c", buf[i]);
			 }				 
			 fclose(fp);
			 free(buf);
		 } else {
			 // 学習失敗
			 fprintf(stderr, "DATA ERROR: Failed to learn\n");
		 }
		 break;

		 // ------------------- P: 表示のみ
	 case 'P':
		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 break; // 失敗したらクローズして終了
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 

		 // P コマンドの送信
		 char P_CMD[6]  =  {'#',',','0','3','\r','\n'};
		 for (j=0; j<6; j++) {
			 question[0] = P_CMD[j];
			 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, question, PACKET_CTRL_LEN,TIMEOUT);
			 // 戻り値が負ならエラー
			 if (r < 0) {
				 fprintf(stderr, "Control Out: libusb_control_transfer error: %s\n",
								 libusb_error_text(r)); goto outerror;
			 }
		 }

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);

		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 // 失敗したらクローズして終了
			 libusb_close(devh); 
			 libusb_exit(NULL);
			 exit(1);
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 
		 
		 // コントロール転送(デバイス→ホスト)
		 do {
			 r = libusb_control_transfer(devh,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, answer,PACKET_CTRL_LEN, TIMEOUT); 
			 if (r < 0) {
				 libusb_release_interface(devh, 0);
				 fprintf(stderr, "Control IN error: %s\n", libusb_error_text(r));
				 exit(1);
			 } 
			 c = answer[0];
			 if (r>0) {
				 if (c=='\r') continue;
				 fprintf(stdout, "%c", c);
			 }
		 } while (c != '\n');

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);
		 break;

		 // ------------------- V: バージョン情報表示
	 case 'V':
		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 break; // 失敗したらクローズして終了
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 

		 // V コマンドの送信
		 char V_CMD[6]  =  {'#',',','0','6','\r','\n'};
		 for (j=0; j<6; j++) {
			 question[0] = V_CMD[j];
			 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, question, PACKET_CTRL_LEN,TIMEOUT);
			 // 戻り値が負ならエラー
			 if (r < 0) {
				 fprintf(stderr, "Control Out: libusb_control_transfer error: %s\n",
								 libusb_error_text(r)); goto outerror;
			 }
		 }

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);

		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 // 失敗したらクローズして終了
			 libusb_close(devh); 
			 libusb_exit(NULL);
			 exit(1);
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 
		 
		 // コントロール転送(デバイス→ホスト)
		 do {
			 r = libusb_control_transfer(devh,CTRL_IN,HID_GET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, answer,PACKET_CTRL_LEN, TIMEOUT); 
			 if (r < 0) {
				 libusb_release_interface(devh, 0);
				 fprintf(stderr, "Control IN error: %s\n", libusb_error_text(r));
				 exit(1);
			 } 
			 c = answer[0];
			 if (r>0) {
				 if (c=='\r') continue;
				 fprintf(stdout, "%c", c);
			 }
		 } while (c != '\n');

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);
		 break;

		 // ------------------- s: EEPROM に保存
	 case 's':
		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 break; // 失敗したらクローズして終了
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 

		 // s コマンドの送信
		 char s_CMD[6]  =  {'#',',','0','5','\r','\n'};
		 for (j=0; j<6; j++) {
			 question[0] = s_CMD[j];
			 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, question, PACKET_CTRL_LEN,TIMEOUT);
			 // 戻り値が負ならエラー
			 if (r < 0) {
				 fprintf(stderr, "Control Out error: %s\n", libusb_error_text(r)); goto outerror;
			 }
		 }

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);
		 break;
		 
		 // ------------------- l: EEPROM から読む
	 case 'l':
		 // インタフェースの要求
		 r = libusb_claim_interface(devh, 0); 
		 if (r < 0) { 
			 fprintf(stderr, "libusb_claim_interface error: %s\n", libusb_error_text(r)); 
			 break; // 失敗したらクローズして終了
		 } 
		 if (opt_debug) fprintf(stderr,"libusb_claim_interface OK\n"); 

		 // l コマンドの送信
		 char l_CMD[6]  =  {'#',',','0','4','\r','\n'};
		 for (j=0; j<6; j++) {
			 question[0] = l_CMD[j];
			 r = libusb_control_transfer(devh,CTRL_OUT,HID_SET_REPORT,(HID_REPORT_TYPE_FEATURE<<8)|0x00,
																	 0, question, PACKET_CTRL_LEN,TIMEOUT);
			 // 戻り値が負ならエラー
			 if (r < 0) {
					fprintf(stderr, "Control Out error: %s\n", libusb_error_text(r)); goto outerror;
				}
		 }

		 // インタフェースの解放
		 libusb_release_interface(devh, 0);
		 break;
		 
		 // ------------------- それ以外
	 } // switch()

   // USB デバイスのリセットと終了
   // libusb_reset_device(devh); 
 outerror:
   libusb_close(devh); 
   libusb_exit(NULL); 
   return 0;
} // main()
