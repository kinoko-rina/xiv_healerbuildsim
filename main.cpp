#include <iostream>
#include <fstream>
#include <math.h>
#include <string>

using namespace std;

#define MATERIA1_VALUE 36	//上位マテリアの数値（アルテマテリジャ+36）
#define MATERIA2_VALUE 12	//下位マテリアの数値（オメガマテリジャ+12）

// 各ステータスの初期値
#define INIT_CRI 400
#define INIT_DH 400
#define INIT_DET 390
#define INIT_SS 400
#define INIT_PIE 390

// Player,Equipmentのstatに対応
#define CRI 0
#define DH 1
#define DET 2
#define SS 3
#define PIE 4

// Equipmentのareaに対応
#define WEAPON 0
#define HEAD 1
#define BODY 2
#define HANDS 3
#define LEGS 4
#define FEET 5
#define EARINGS 6
#define NECKLACE 7
#define BRACELETS 8
#define LRING 9
#define RRING 10



// プレイヤー
struct Player{
	int stat[5];
	double cri_rate = 0.05;
	double cri_dmg = 0.4;
	double cri_avg = 1.02;

	double dh_rate = 0.0;
	double dh_avg = 1.0;

	double det_avg = 1.0;

	double gcd = 2.5;
	double dot_dmg = 1.0;

	double sage_dmg = 0.0;
	double sage_dmg_rate = 0.0;

	void calc_stat(){
		cri_rate = (50 + floor(200 * (stat[CRI] - 400) / 1900)) / 1000;
		cri_dmg = (1400 + floor(200 * (stat[CRI] - 400) / 1900)) / 1000;
		cri_avg = cri_rate * (cri_dmg - 1.0) + 1.0;
		dh_rate = floor(550 * (stat[DH] - 400) / 1900) / 1000;
		dh_avg = dh_rate * 0.25 + 1.0;
		det_avg = (1000 + floor(140 * (stat[DET] - 390) / 1900)) / 1000;
		gcd = floor(2500 * (1000 + ceil(130 * (400 - stat[SS]) / 1900)) / 10000) / 100;
		dot_dmg = (1000 + floor(130 * (stat[SS] - 400) / 1900)) / 1000;
		calc_sage_damage();
	}

	//賢者180秒でのダメージ期待値の計算値（dotを切れ目なく更新）
	double calc_sage_damage_part(double t_time, double t_dot_dmg,double t_gcd, double t_eukrasia_num){
		return (70.0/3.0 * t_dot_dmg) * t_time +	//エウクラシア・ドシスIII (dot)
			330.0 * ((t_time - 2.5 * ceil(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*ceil(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//ドシスIII
			510.0 * (t_time / 45.0);	//フレグマ
	}

	//賢者180秒でのダメージ期待値の計算値（dotを切らして更新）
	double calc_sage_damage_part2(double t_time, double t_dot_dmg, double t_gcd, double t_eukrasia_num){
		return (70.0 / 3.0 * t_dot_dmg) * floor(t_time / 30.0) * 30.0 +	//エウクラシア・ドシスIII (dot)
			330.0 * ((t_time - 2.5 * floor(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*floor(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//ドシスIII
			510.0 * (t_time / 45.0);	//フレグマ
	}

	//賢者180秒でのダメージ期待値の計算値（MIN値＝dot更新のロスが最大）
	double calc_sage_damage_part3min(double t_time, double t_dot_dmg, double t_gcd, double t_eukrasia_num){
		return (70.0 / 3.0 * t_dot_dmg) * floor(t_time / 30.0) * 28.75 +	//エウクラシア・ドシスIII (dot)
			330.0 * ((t_time - 2.5 * floor(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*floor(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//ドシスIII
			510.0 * (t_time / 45.0);	//フレグマ
	}


	//賢者180秒でのダメージ期待値（倍率）
	double calc_sage_damage(){
		const double eukrasia_num = 0.5; //30秒単位で、dot付与以外でエウクラシアを使う数
		const double basedamage = calc_sage_damage_part(180.0, 1.0, 2.5, eukrasia_num) * 1.02;

		/* 
		//dot更新タイミングまで含め、詳細に計算する
		double calc_time1 = 180.0 - ((27.5 - 2.5 * eukrasia_num) - gcd * floor((27.5 - 2.5 * eukrasia_num) / gcd)) * 6.0;
		double t_sage_dmg1 = calc_sage_damage_part(calc_time1, dot_dmg, gcd, eukrasia_num);
		double calc_time2 = 180.0 - ((27.5 - 2.5 * eukrasia_num) - gcd * ceil((27.5 - 2.5 * eukrasia_num) / gcd)) * 6.0;
		double t_sage_dmg2 = calc_sage_damage_part2(calc_time2, dot_dmg, gcd, eukrasia_num);
		double result1 = (t_sage_dmg1* cri_avg * dh_avg * det_avg) / (basedamage * calc_time1 / 180.0);
		double result2 = (t_sage_dmg2* cri_avg * dh_avg * det_avg) / (basedamage * calc_time2 / 180.0);
		if (result1 > result2){ sage_dmg = t_sage_dmg1; return result1; }
		else{ sage_dmg = t_sage_dmg2; return result2; }
		*/

		//dotロスを考慮した最大パターンと最小パターンを足して2で割る計算方法
		sage_dmg = (calc_sage_damage_part(180.0, dot_dmg, gcd, eukrasia_num) + calc_sage_damage_part3min(180.0, dot_dmg, gcd, eukrasia_num)) / 2.0 * cri_avg * dh_avg * det_avg;
		sage_dmg_rate = sage_dmg / basedamage;
		return sage_dmg_rate;

	}

	void reset(){
		stat[CRI] = INIT_CRI;
		stat[DH] = INIT_DH;
		stat[DET] = INIT_DET;
		stat[SS] = INIT_SS;
		stat[PIE] = INIT_PIE;
	}

	Player(){
		reset();
	};

};

// マテリア装着データ
struct MateriaData{
	int slotinfo[5]; // 5つあるマテリア穴の情報 0穴なし　1上位マテリア穴　2下位マテリア穴
	int statmax[5]; // サブステごとのサブステ上限値

	int slot[5]; // マテリア穴にどのマテリアが刺さっているか CRI0 DH1 DET2 SS3 PIE4
	int stat[5]; // サブステごとに実際に上昇するステータス

	void reset(){
		for (int i = 0; i < 5; i++){
			slotinfo[i] = 0;
			statmax[i] = 0;
			slot[i] = -1;
			stat[i] = 0;
		}
	}

	MateriaData(){
		reset();
	};
};

// 装備
struct Equipment{
	string name; // 装備名
	int enable;
	int area; // 装備部位
	int stat[5]; // 装備についているサブステの値
	int statmax; // サブステ上限値
	int materia1_slotnum; // 上位マテリア穴の数
	int materia2_slotnum; // 下位マテリア穴の数
	void set(string t_name, int t_area, int t_cri, int t_dh, int t_det, int t_ss, int t_pie, int m_slot1num, int m_slot2num, int t_statmax){
		name = t_name;
		enable = true;
		area = t_area;
		stat[CRI] = t_cri;
		stat[DH] = t_dh;
		stat[DET] = t_det;
		stat[SS] = t_ss;
		stat[PIE] = t_pie;
		materia1_slotnum = m_slot1num;
		materia2_slotnum = m_slot2num;
		statmax = t_statmax;
	}
	Equipment(string t_name,int t_area, int t_cri, int t_dh, int t_det, int t_ss, int t_pie, int m_slot1num, int m_slot2num,int t_statmax){
		set(t_name, t_area, t_cri, t_dh, t_det, t_ss, t_pie, m_slot1num, m_slot2num, t_statmax);
	};
	Equipment(){ enable = false; };
};

// 食事
struct Food{
	string name; // 食事名
	int enable;
	double stat_p[5]; // ステータス倍率
	int stat_max[5]; // ステータスの上限値
	void set(string t_name, double d_cri, double d_dh, double d_det, double d_ss, double d_pie, int max_cri, int max_dh, int max_det, int max_ss, int max_pie){
		name = t_name;
		enable = true;
		stat_p[CRI] = d_cri;
		stat_p[DH] = d_dh;
		stat_p[DET] = d_det;
		stat_p[SS] = d_ss;
		stat_p[PIE] = d_pie;
		stat_max[CRI] = max_cri;
		stat_max[DH] = max_dh;
		stat_max[DET] = max_det;
		stat_max[SS] = max_ss;
		stat_max[PIE] = max_pie;
	}
	Food(string t_name, double d_cri, double d_dh, double d_det, double d_ss, double d_pie, int max_cri, int max_dh, int max_det, int max_ss, int max_pie){
		set(t_name, d_cri, d_dh, d_det, d_ss, d_pie, max_cri, max_dh, max_det, max_ss, max_pie);
	};
	Food(){ enable = false; };
};


// マテリアが実際に装着可能かをチェックする
int MateriaCheck(MateriaData (&m)[11], int m1_num[5], int m2_num[5]){

	int check = 0; // 戻り値用。上限値オーバーや未装着があった場合、check=1に変える

	// 上位マテリアのチェック。合計装着個数ぶんループする
	int all_m1_num = m1_num[0] + m1_num[1] + m1_num[2] + m1_num[3] + m1_num[4]; //上位マテリアを装着する合計数
	for (int temp = 0; temp < all_m1_num; temp++){

		// ここからどのサブステのマテリアをはめるか決定する(装着数と装着可能場所の差(dif)が小さいサブステを優先する)
		int dif_min = 99;
		int now_st = -1;
		for (int st = 0; st < 5; st++){
			if (m1_num[st] == 0){ continue; } // マテリア装着予定数0の場合は次のサブステへスキップ
			int t_n = 0; // そのマテリアが装着できる最大数
			for (int area = 0; area < 11; area++){
				int t_statmax = m[area].statmax[st];
				for (int i = 0; i < 5; i++){
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 1){
						if ((t_statmax + MATERIA1_VALUE - 1) / MATERIA1_VALUE > 0){
							t_n++;
							t_statmax -= MATERIA1_VALUE;
						}
						else{
							
						}
					}
				}
			}
			if (t_n == 0){ 
				//cout << "(M1)stat[" << st << "]x" << m1_num[st] << "個 空きがなく装着できず"<<endl;
				check = 1;
				continue;
			} // マテリア装着できる場所が0の場合は次のサブステへスキップ
			int dif = t_n - m1_num[st];
			//cout << temp << "_dif(" << st << ")=" << dif << endl;
			if (dif < dif_min){ dif_min = dif; now_st = st; }
		}
		if (now_st == -1){ break; } // すでにはめられるマテリア無し（上限オーバー）
		// ここまででnow_stが決定（どのサブステのマテリアか決定）


		// ここからどのareaの穴にマテリアをはめるか決定する(上限値(st_max)に最も余裕のあるareaの穴に装着する)
		int st_max = 0;
		int now_area = -1;
		for (int area = 0; area < 11; area++){
			if (st_max < m[area].statmax[now_st]){
				for (int i = 0; i < 5; i++){
					// そのareaに1つでも、空きのある上位マテリア穴が存在すればok
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 1){
						now_area = area;
						st_max = m[area].statmax[now_st];
						break;
					}
				}
			}
		}
		// ここまででnow_areaが決定（マテリアを装着する部位決定）


		// ここからマテリアを装着する処理
		for (int i = 0; i < 5; i++){
			// 空きのある上位マテリア穴
			if (m[now_area].slot[i] == -1 && m[now_area].slotinfo[i] == 1){
				m[now_area].slot[i] = now_st;
				m1_num[now_st]--;
				if (m[now_area].statmax[now_st] < MATERIA1_VALUE){ // 上限値オーバーで装着
					//cout << "(M1)area" << now_area << "のスロット[" << i << "]に装着->stat[" << now_st << "]+" << m[now_area].statmax[now_st] << "(over!)" << endl;
					m[now_area].stat[now_st] += m[now_area].statmax[now_st];
					m[now_area].statmax[now_st] = 0;
					check = 1;
				}
				else{ // 正常に装着
					//cout << "(M1)area" <<  now_area << "のスロット[" << i << "]に装着->stat[" << now_st << "] + " << MATERIA1_VALUE << endl;
					m[now_area].stat[now_st] += MATERIA1_VALUE;
					m[now_area].statmax[now_st] -= MATERIA1_VALUE;
				}
				break;
			}
		}
		// ここまでマテリア装着処理

	}
	// 上位マテリアの処理完了


	// 下位マテリアのチェック。合計装着個数ぶんループする
	int all_m2_num = m2_num[0] + m2_num[1] + m2_num[2] + m2_num[3] + m2_num[4]; //上位マテリアを装着する合計数
	for (int temp = 0; temp < all_m2_num; temp++){

		// ここからどのサブステのマテリアをはめるか決定する(装着数と装着可能場所の差(dif)が小さいサブステを優先する)
		int dif_min = 99;
		int now_st = -1;
		for (int st = 0; st < 5; st++){
			if (m2_num[st] == 0){ continue; } // マテリア装着予定数0の場合は次のサブステへスキップ
			int t_n = 0; // そのマテリアがさせる最大数
			for (int area = 0; area < 11; area++){
				int t_statmax = m[area].statmax[st];
				for (int i = 0; i < 5; i++){
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 2){
						if ((t_statmax + MATERIA2_VALUE - 1) / MATERIA2_VALUE > 0){
							t_n++;
							t_statmax -= MATERIA2_VALUE;
						}
						else{
							//
						}
					}
				}
			}
			if (t_n == 0){
				//cout << "(M2)stat[" << st << "]x" << m2_num[st] << "個 空きがなく装着できず"<< endl;
				check = 1;
				continue;
			} // マテリア装着できる場所が0の場合は次のサブステへスキップ
			int dif = t_n - m2_num[st];
			//cout << "dif2(" << st << ")=" << dif << endl;
			if (dif < dif_min){ dif_min = dif; now_st = st; }
		}
		if (now_st == -1){ break; } // すでにはめられるマテリア無し（上限オーバー）
		// ここまででnow_stが決定（どのサブステのマテリアか決定）


		// ここからどのareaの穴にマテリアをはめるか決定する(上限値(st_max)に最も余裕のあるareaの穴に装着する)
		int st_max = 0;
		int now_area = -1;
		for (int area = 0; area < 11; area++){
			if (st_max < m[area].statmax[now_st]){
				for (int i = 0; i < 5; i++){
					// そのareaに1つでも、空きのある下位マテリア穴が存在すればok
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 2){
						now_area = area;
						st_max = m[area].statmax[now_st];
						break;
					}
				}
			}
		}
		// ここまででnow_areaが決定（マテリアを装着する部位決定）


		// ここからマテリアを装着する処理
		for (int i = 0; i < 5; i++){
			// 空きのある下位マテリア穴
			if (m[now_area].slot[i] == -1 && m[now_area].slotinfo[i] == 2){
				m[now_area].slot[i] = now_st;
				m2_num[now_st]--;
				if (m[now_area].statmax[now_st] < MATERIA2_VALUE){ // 上限値オーバーで装着
					//cout << "(M2)area" << now_area << "のスロット[" << i << "]に装着->stat[" << now_st << "]+" << m[now_area].statmax[now_st] << "(over!)" << endl;
					m[now_area].stat[now_st] += m[now_area].statmax[now_st];
					m[now_area].statmax[now_st] = 0;
					check = 1;
				}
				else{ // 正常に装着
					//cout << "(M2)area" <<  now_area << "のスロット[" << i << "]に装着->stat[" << now_st << "] + " << MATERIA2_VALUE << endl;
					m[now_area].stat[now_st] += MATERIA2_VALUE;
					m[now_area].statmax[now_st] -= MATERIA2_VALUE;
				}
				break;
			}
		}
		// ここまでマテリア装着処理

	}
	// 下位マテリアの処理完了

	return check;
}

int main(){

	cin.exceptions(ios::failbit);

	Equipment equip[11][4]; // 装備
	Food food[5];

	// 装備ファイルの読み込み
	ifstream equip_file;
	equip_file.open("equip.txt", ios::in);
	if (equip_file){
		string line;
		while (getline(equip_file, line)){
			if (line.empty() || line[0] == '#') { 
				continue;
			}
			char t_name[99];
			int t_area;
			int t_cri;
			int t_dh;
			int t_det;
			int t_ss;
			int t_pie;
			int t_m1slotnum;
			int t_m2slotnum;
			int t_statmax;
			sscanf_s(line.c_str(), "%[^\n,],%d,%d,%d,%d,%d,%d,%d,%d,%d", t_name, sizeof(t_name), &t_area, &t_cri, &t_dh, &t_det, &t_ss, &t_pie, &t_m1slotnum, &t_m2slotnum, &t_statmax);
			if (t_area >= 11) { continue; } 
			for (int i = 0; i < 4; i++){
				if (equip[t_area][i].enable == false){
					equip[t_area][i].set(t_name, t_area, t_cri, t_dh, t_det, t_ss, t_pie, t_m1slotnum, t_m2slotnum, t_statmax);
					break;
				}
			}
		}
	}
	else{
		return -1;
	}
	equip_file.close();

	// 食事ファイルの読み込み
	ifstream food_file;
	food_file.open("food.txt", ios::in);
	if (food_file){
		string line;
		while (getline(food_file, line)){
			if (line.empty() || line[0] == '#') {
				continue;
			}
			char t_name[99];
			double d_cri;
			double d_dh;
			double d_det;
			double d_ss;
			double d_pie;
			int max_cri;
			int max_dh;
			int max_det;
			int max_ss;
			int max_pie;
			sscanf_s(line.c_str(), "%[^\n,],%lf,%lf,%lf,%lf,%lf,%d,%d,%d,%d,%d", t_name, sizeof(t_name), &d_cri, &d_dh, &d_det, &d_ss, &d_pie, &max_cri, &max_dh, &max_det, &max_ss, &max_pie);
			for (int i = 0; i < 5; i++){
				if (food[i].enable == false){
					food[i].set(t_name, d_cri, d_dh, d_det, d_ss, d_pie, max_cri, max_dh, max_det, max_ss, max_pie);
					break;
				}
			}
		}
	}
	else{
		return -1;
	}
	food_file.close();

	/*
	//サブステ順番は0.Cri 1.DH 2.Det 3.SS 4.Pie
	equip[0][0].set("武器|新式", WEAPON, 0, 0, 0, 253, 177, 3, 2, 253);
	equip[0][1].set("武器|極　", WEAPON, 177, 0, 253, 0, 0, 2, 0, 253);
	equip[1][0].set("頭　|新式", HEAD, 0, 0, 108, 154, 0, 3, 2, 154);
	//equip[1][1].set("頭　|N 　", HEAD, 0, 0, 108, 154, 0, 2, 0, 154);
	equip[2][0].set("胴　|新式", BODY, 0, 0, 244, 171, 0, 3, 2, 244);
	equip[2][1].set("胴　|N 　", BODY, 244, 0, 171, 0, 0, 2, 0, 244);
	equip[3][0].set("手　|新式", HANDS, 154, 108, 0, 0, 0, 3, 2, 154);
	//equip[3][1].set("手　|N 　", HANDS, 0, 0, 154, 0, 108, 2, 0, 154);
	equip[4][0].set("脚　|新式", LEGS, 0, 0, 0, 244, 171, 3, 2, 244);
	equip[4][1].set("脚　|N 　", LEGS, 244, 0, 171, 0, 0, 2, 0, 244);
	equip[5][0].set("足　|新式", FEET, 108, 0, 154, 0, 0, 3, 2, 154);
	//equip[5][1].set("足　|N 　", FEET, 0, 0, 154, 108, 0, 2, 0, 154);
	equip[6][0].set("耳　|新式", EARINGS, 121, 0, 0, 0, 85, 2, 3, 121);
	equip[7][0].set("首　|新式", NECKLACE, 0, 0, 121, 85, 0, 2, 3, 121);
	equip[8][0].set("腕　|新式", BRACELETS, 0, 0, 85, 0, 121, 2, 3, 121);
	equip[9][0].set("指1 |新式", LRING, 121, 0, 0, 0, 85, 2, 3, 121);
	equip[10][0].set("指2 |新式", RRING, 0, 0, 85, 121, 0, 2, 3, 121);

	//food[0].set("なし", 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
	food[0].set("ココナッツコッドチャウダーHQ", 0.1, 0.0, 0.1, 0.0, 0.0, 46, 0, 76, 0, 0);
	food[1].set("ババロア・オ・シューコンHQ", 0.0, 0.1, 0.1, 0.0, 0.0, 0, 82, 50, 0, 0);
	*/

	Player p_init;

	int m1_slotnum = 0; // 上位マテリアが嵌る穴の数
	int m2_slotnum = 0; // 下位マテリアが嵌る穴の数
	int m1_maxnum[5] = { 0, 0, 0, 0, 0 }; // 上限値を考慮した、ステータスごとの上位マテリアが嵌る穴の数
	int m2_maxnum[5] = { 0, 0, 0, 0, 0 }; // 上限値を考慮した、ステータスごとの下位マテリアが嵌る穴の数
	MateriaData equip_m_init[11]; // 部位ごとのマテリア情報 初期値
	int selected_equip[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //部位ごとに選んだ装備

	// 最大となる結果を保存するための変数群
	Player p_max;
	MateriaData equip_m_max[11];
	int food_maxnum = 0;
	double d_max = 0.0;
	int selected_equip_max[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


	// 装備のパターン数を計算（後のループで使用）
	int max_equip_pattern = 0;
	for (int i0 = 0; i0 < 4; i0++){ if (equip[0][i0].enable == false){ break; }
	for (int i1 = 0; i1 < 4; i1++){ if (equip[1][i1].enable == false){ break; }
	for (int i2 = 0; i2 < 4; i2++){ if (equip[2][i2].enable == false){ break; }
	for (int i3 = 0; i3 < 4; i3++){ if (equip[3][i3].enable == false){ break; }
	for (int i4 = 0; i4 < 4; i4++){ if (equip[4][i4].enable == false){ break; }
	for (int i5 = 0; i5 < 4; i5++){ if (equip[5][i5].enable == false){ break; }
	for (int i6 = 0; i6 < 4; i6++){ if (equip[6][i6].enable == false){ break; }
	for (int i7 = 0; i7 < 4; i7++){ if (equip[7][i7].enable == false){ break; }
	for (int i8 = 0; i8 < 4; i8++){ if (equip[8][i8].enable == false){ break; }
	for (int i9 = 0; i9 < 4; i9++){ if (equip[9][i9].enable == false){ break; }
	for (int i10 = 0; i10 < 4; i10++){ if (equip[10][i10].enable == false){ break; }
		max_equip_pattern++;
	}}}}}}}}}}}
	

	// ユーザー入力項目ここから

	// 信仰の必要値　これ以上の信仰を確保するようにする。0の場合、信仰マテリアを0個にするため処理が早くなる
	int set_min_pie = 0;
	cout << "信仰(Pie)の必要値を入力。入力値以上の信仰を確保するようにビルドを組みます。" << endl;
	while (true){
		try{ cin >> set_min_pie; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (set_min_pie < 0) { set_min_pie = 0; }

	// gcdの最小値　これ以上のgcdになるようにする（早すぎるgcdを防止するためのオプション）0.0の場合は制限なし
	double set_gcd_limit = 0.0;
	cout << "GCDの最小値を入力(0.0-2.5)。入力値以上のGCDとなるようにビルドを組みます。" << endl << "（早すぎるGCDを防止したいときに使用してください）" << endl;
	while (true){
		try{ cin >> set_gcd_limit; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (set_gcd_limit > 2.5) { set_min_pie = 2.5; }
	else if (set_gcd_limit < 0.0) { set_min_pie = 0.0; }

	// ユーザー入力項目ここまで


	// loop開始
	for (int loop = 0; loop < max_equip_pattern; loop++){

		//system("cls");
		cout << endl << loop + 1 << "/" << max_equip_pattern << " 計算中...";

	int temp_l = -1;
		for (int i0 = 0; i0 < 4; i0++){ if (equip[0][i0].enable == false){ break; }
		for (int i1 = 0; i1 < 4; i1++){ if (equip[1][i1].enable == false){ break; }
		for (int i2 = 0; i2 < 4; i2++){ if (equip[2][i2].enable == false){ break; }
		for (int i3 = 0; i3 < 4; i3++){ if (equip[3][i3].enable == false){ break; }
		for (int i4 = 0; i4 < 4; i4++){ if (equip[4][i4].enable == false){ break; }
		for (int i5 = 0; i5 < 4; i5++){ if (equip[5][i5].enable == false){ break; }
		for (int i6 = 0; i6 < 4; i6++){ if (equip[6][i6].enable == false){ break; }
		for (int i7 = 0; i7 < 4; i7++){ if (equip[7][i7].enable == false){ break; }
		for (int i8 = 0; i8 < 4; i8++){ if (equip[8][i8].enable == false){ break; }
		for (int i9 = 0; i9 < 4; i9++){ if (equip[9][i9].enable == false){ break; }
		for (int i10 = 0; i10 < 4; i10++){ if (equip[10][i10].enable == false){ break; }
		if (temp_l < loop){ 
			temp_l++;
			if (temp_l == loop){ 
				selected_equip[0] = i0; selected_equip[1] = i1; selected_equip[2] = i2; selected_equip[3] = i3;
				selected_equip[4] = i4; selected_equip[5] = i5; selected_equip[6] = i6; selected_equip[7] = i7;
				selected_equip[8] = i8; selected_equip[9] = i9; selected_equip[10] = i10;
			}
		}
		}}}}}}}}}}}

	p_init.reset();
	m1_slotnum = 0;
	m2_slotnum = 0;
	for (int area = 0; area < 11; area++){ equip_m_init[area].reset(); }
	for (int st = 0; st < 5; st++){ m1_maxnum[st] = 0; m2_maxnum[st] = 0; }

	// 装備に装着可能なマテリアの個数など、初期情報の準備。装備部位ごとに処理を行う
	for (int area = 0; area < 11; area++){

		m1_slotnum += equip[area][selected_equip[area]].materia1_slotnum;
		m2_slotnum += equip[area][selected_equip[area]].materia2_slotnum;

		// マテリアのスロット情報を代入
		for (int i = 0; i < equip[area][selected_equip[area]].materia1_slotnum; i++){
			equip_m_init[area].slotinfo[i] = 1;
		}
		for (int i = equip[area][selected_equip[area]].materia1_slotnum; i < equip[area][selected_equip[area]].materia1_slotnum + equip[area][selected_equip[area]].materia2_slotnum; i++){
			equip_m_init[area].slotinfo[i] = 2;
		}

		for (int st = 0; st < 5; st++){
			p_init.stat[st] += equip[area][selected_equip[area]].stat[st];
			int st_temp = equip[area][selected_equip[area]].statmax - equip[area][selected_equip[area]].stat[st];
			if (st_temp == 0){ equip_m_init[area].statmax[st] = 0; continue; }
			if (st_temp > equip[area][selected_equip[area]].materia1_slotnum*MATERIA1_VALUE + equip[area][selected_equip[area]].materia2_slotnum*MATERIA2_VALUE){
				st_temp = equip[area][selected_equip[area]].materia1_slotnum*MATERIA1_VALUE + equip[area][selected_equip[area]].materia2_slotnum*MATERIA2_VALUE;
				m1_maxnum[st] += equip[area][selected_equip[area]].materia1_slotnum;
				m2_maxnum[st] += equip[area][selected_equip[area]].materia2_slotnum;
			}
			else{
				if ((st_temp + MATERIA1_VALUE - 1) / MATERIA1_VALUE < equip[area][selected_equip[area]].materia1_slotnum){
					m1_maxnum[st] += (st_temp + MATERIA1_VALUE - 1) / MATERIA1_VALUE;
				}
				else{
					m1_maxnum[st] += equip[area][selected_equip[area]].materia1_slotnum;
				}
				if ((st_temp + MATERIA2_VALUE - 1) / MATERIA2_VALUE < equip[area][selected_equip[area]].materia2_slotnum){
					m2_maxnum[st] += (st_temp + MATERIA2_VALUE - 1) / MATERIA2_VALUE;
				}
				else{
					m2_maxnum[st] += equip[area][selected_equip[area]].materia2_slotnum;
				}
			}
			equip_m_init[area].statmax[st] = st_temp;
		}
	}

	// 一度計算
	p_init.calc_stat();

	// 素のステータスで既にGCDがリミットを超えていた場合、NG
	if (set_gcd_limit > p_init.gcd){ cout << "(skip)"; continue; }

	int check_pie = 1; // loop中にpie制限をチェックするかどうか(1でチェックする)
	// PIE制限がない場合、PIEマテリアの個数を0個に設定しておく
	if (set_min_pie == 0) {
		m1_maxnum[4] = 0;
		m2_maxnum[4] = 0;
		check_pie = 0;
	}
	// 素のステータスで既に必要PIEを超えていた場合も、PIEマテリアの個数を0個に設定
	else if (set_min_pie < p_init.stat[PIE]){
		m1_maxnum[4] = 0;
		m2_maxnum[4] = 0;
		check_pie = 0;
	}


	// すべての上位マテリア個数パターンをシミュレートする
	int s1[5] = { 0, 0, 0, 0, 0 }; // ステータスごとの上位マテリアの装着個数
	for (s1[0] = 0; s1[0] <= m1_slotnum && s1[0] <= m1_maxnum[0]; s1[0]++){
		for (s1[1] = 0; s1[1] <= m1_slotnum - s1[0] && s1[1] <= m1_maxnum[1]; s1[1]++){
			for (s1[2] = 0; s1[2] <= m1_slotnum - s1[0] - s1[1] && s1[2] <= m1_maxnum[2]; s1[2]++){
				for (s1[3] = 0; s1[3] <= m1_slotnum - s1[0] - s1[1] - s1[2] && s1[3] <= m1_maxnum[3]; s1[3]++){
					s1[4] = m1_slotnum - s1[0] - s1[1] - s1[2] - s1[3];
					if (s1[4] > m1_maxnum[4]) { continue; }

					// すべての下位マテリア個数パターンをシミュレートする
					int s2[5] = { 0, 0, 0, 0, 0 }; // ステータスごとの下位マテリアの装着個数
					for (s2[0] = 0; s2[0] <= m2_slotnum && s2[0] <= m2_maxnum[0]; s2[0]++){
						for (s2[1] = 0; s2[1] <= m2_slotnum - s2[0] && s2[1] <= m2_maxnum[1]; s2[1]++){
							for (s2[2] = 0; s2[2] <= m2_slotnum - s2[0] - s2[1] && s2[2] <= m2_maxnum[2]; s2[2]++){
								for (s2[3] = 0; s2[3] <= m2_slotnum - s2[0] - s2[1] - s2[2] && s2[3] <= m2_maxnum[3]; s2[3]++){
									s2[4] = m2_slotnum - s2[0] - s2[1] - s2[2] - s2[3];
									if (s2[4] > m2_maxnum[4]) { continue; }

									for (int f = 0; f < 5; f++){ // 最適な食事を選択
										if (food[f].enable == false){ continue; }

										Player p = p_init;
										for (int st = 0; st < 5; st++){
											p.stat[st] += s1[st] * MATERIA1_VALUE + s2[st] * MATERIA2_VALUE; // マテリア分のサブステ加算
											int food_plus = p.stat[st] * food[f].stat_p[st]; // 食事分の増加計算
											if (food_plus>food[f].stat_max[st]){ food_plus = food[f].stat_max[st]; }
											p.stat[st] += food_plus;
										}
										if (check_pie == 1) {
											if (set_min_pie > p.stat[PIE]){ continue; } // 信仰が必要値に満たなかった場合はNG
											if (set_min_pie + MATERIA1_VALUE < p.stat[PIE]){ if (s1[4]>0 || s2[4] > 0) { continue; } } // マテリアを使用している場合、信仰が必要値より大きくオーバーしている場合もNG
										}
										p.calc_stat(); // 期待値計算
										if (p.sage_dmg_rate > d_max){ // 最大値更新時

											// マテリア装着が可能かチェック
											MateriaData equip_m[11];
											for (int i = 0; i < 11; i++){ equip_m[i] = equip_m_init[i]; }
											int m1_num[5] = { 0, 0, 0, 0, 0 };
											int m2_num[5] = { 0, 0, 0, 0, 0 };
											for (int i = 0; i < 5; i++){
												m1_num[i] = s1[i];
												m2_num[i] = s2[i];
											}
											if (MateriaCheck(equip_m, m1_num, m2_num) == 0){ // マテリア装着がok
												if (set_gcd_limit > p.gcd){ continue; } // gcdがリミットを超えてしまった場合はNG
												d_max = p.sage_dmg_rate;
												p_max = p;
												food_maxnum = f;
												for (int i = 0; i < 11; i++){ equip_m_max[i] = equip_m[i]; selected_equip_max[i] = selected_equip[i]; }
												//cout << s1[0] << "," << s1[1] << "," << s1[2] << "," << s1[3] << "," << s1[4] << "-" << s2[0] << "," << s2[1] << "," << s2[2] << "," << s2[3] << "," << s2[4] << "-> 倍率:" << p.sage_dmg_rate * 100 << "％" << endl;
											}
											else{ //マテリア装着に上限オーバーor未装着があった場合、一度初期値に戻した後マテリア分を増加させ再計算
												p = p_init;
												for (int s = 0; s < 5; s++){
													for (int a = 0; a < 11; a++){
														p.stat[s] += equip_m[a].stat[s];
													}
													int food_plus = p.stat[s] * food[f].stat_p[s]; // 食事分の増加計算
													if (food_plus>food[f].stat_max[s]){ food_plus = food[f].stat_max[s]; }
													p.stat[s] += food_plus;
												}
												if (set_min_pie > p.stat[PIE]){ continue; } // 信仰が必要値に満たなかった場合はNG
												p.calc_stat(); // 改めて期待値計算
												if (p.sage_dmg_rate > d_max){ // それでも最大値が更新された場合
													if (set_gcd_limit > p.gcd){ continue; } // gcdがリミットを超えてしまった場合はNG
													d_max = p.sage_dmg_rate;
													p_max = p;
													food_maxnum = f;
													for (int i = 0; i < 11; i++){ equip_m_max[i] = equip_m[i]; selected_equip_max[i] = selected_equip[i]; }
													//cout << s1[0] << "," << s1[1] << "," << s1[2] << "," << s1[3] << "," << s1[4] << "-" << s2[0] << "," << s2[1] << "," << s2[2] << "," << s2[3] << "," << s2[4] << "-> 倍率:" << p.sage_dmg_rate * 100 << "％ *マテリア上限オーバーで更新" << endl;
												}
											}
										}
									}
								}
							}
						}
					}


				}
			}
		}
	}

	}
	// loop終了

	// 結果表示の準備
	int m1_nummax[5] = { 0, 0, 0, 0, 0 };
	int m2_nummax[5] = { 0, 0, 0, 0, 0 };
	int m_stmax[5] = { 0, 0, 0, 0, 0 };
	for (int st = 0; st < 5; st++){
		for (int i = 0; i < 11; i++){
			m_stmax[st] += equip_m_max[i].stat[st];
			for (int s = 0; s < 5; s++){
				if (equip_m_max[i].slot[s] == st && equip_m_max[i].slotinfo[s] == 1){ m1_nummax[st]++; }
				if (equip_m_max[i].slot[s] == st && equip_m_max[i].slotinfo[s] == 2){ m2_nummax[st]++; }
			}
		}
	}

	// 結果の表示
	cout << endl << endl << "====結果表示====" << endl;
	if (set_min_pie>0){ cout << "追加条件：信仰" << set_min_pie << " 以上" << endl; }
	if (set_gcd_limit>0){ cout << "追加条件：GCD" << set_gcd_limit << "s 以上" << endl; }
	cout << "【装備詳細】" << endl;
	for (int i = 0; i < 11; i++){
		cout << "　" << equip[i][selected_equip_max[i]].name << "|";
		for (int s = 0; s < 5; s++){
			if (equip_m_max[i].slotinfo[s] != -1){ 
				switch (equip_m_max[i].slot[s]){
					case 0: cout << "武略"; break;
					case 1: cout << "天眼"; break;
					case 2: cout << "雄略"; break;
					case 3: cout << "詠唱"; break;
					case 4: cout << "信力"; break;
				}
				switch (equip_m_max[i].slotinfo[s]){
					case 1: cout << "ｱﾙﾃﾏ　"; break;
					case 2: cout << "ｵﾒｶﾞ　"; break;
				}
			}
		}
		cout << endl;
	}
	cout << "【食事】" << food[food_maxnum].name << endl;
	cout << "【ステータス】" << endl << "　Cri  " << p_max.stat[CRI] << " （クリ率" << p_max.cri_rate * 100 << "％、倍率" << p_max.cri_dmg * 100 << "％）" << endl
		<< "　DH   " << p_max.stat[DH] << " （DH率" << p_max.dh_rate * 100 << "％）" << endl
		<< "　意思 " << p_max.stat[DET] << endl
		<< "　SS   " << p_max.stat[SS] << " （" << "GCD " << p_max.gcd << "s、dot倍率" << p_max.dot_dmg * 100 << "％）" << endl
		<< "　信仰 " << p_max.stat[PIE] << endl
		<< "　（マテリア情報）武略ｱﾙﾃﾏx" << m1_nummax[0] << "、天眼ｱﾙﾃﾏx" << m1_nummax[1] << "、雄略ｱﾙﾃﾏx" << m1_nummax[2] << "、詠唱ｱﾙﾃﾏx" << m1_nummax[3] << "、信力ｱﾙﾃﾏx" << m1_nummax[4] << endl
		<< "　　　　　　　　　武略ｵﾒｶﾞx" << m2_nummax[0] << "、天眼ｵﾒｶﾞx" << m2_nummax[1] << "、雄略ｵﾒｶﾞx" << m2_nummax[2] << "、詠唱ｵﾒｶﾞx" << m2_nummax[3] << "、信力ｵﾒｶﾞx" << m2_nummax[4] << endl
		<< "　　　　　　　　　マテリアによるステータス：Cri+" << m_stmax[0] << " DH+" << m_stmax[1] << " Det+" << m_stmax[2] << " SS+" << m_stmax[3] << " Pie+" << m_stmax[4] << endl
		<< "【総合ダメージ倍率】" << p_max.sage_dmg_rate * 100 << "％" << endl;


	int if_out = 0;
	cout << endl  << "output.txtへ結果出力を行いますか？(1で出力を行います)" << endl;
	while (true){
		try{ cin >> if_out; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (if_out == 1) {
		// ファイル出力を行う
		ofstream fout;
		fout.open("output.txt");
			fout << "====結果表示====" << endl;
			if (set_min_pie>0){ fout << "追加条件：信仰" << set_min_pie << " 以上" << endl; }
			if (set_gcd_limit>0){ fout << "追加条件：GCD" << set_gcd_limit << "s 以上" << endl; }
			fout << "【装備詳細】" << endl;
			for (int i = 0; i < 11; i++){
				fout << "　" << equip[i][selected_equip_max[i]].name << "|";
				for (int s = 0; s < 5; s++){
					if (equip_m_max[i].slotinfo[s] != -1){ 
						switch (equip_m_max[i].slot[s]){
							case 0: fout << "武略"; break;
							case 1: fout << "天眼"; break;
							case 2: fout << "雄略"; break;
							case 3: fout << "詠唱"; break;
							case 4: fout << "信力"; break;
						}
						switch (equip_m_max[i].slotinfo[s]){
							case 1: fout << "ｱﾙﾃﾏ　"; break;
							case 2: fout << "ｵﾒｶﾞ　"; break;
						}
					}
				}
				fout << endl;
			}
			fout << "【食事】" << food[food_maxnum].name << endl;
			fout << "【ステータス】" << endl << "　Cri  " << p_max.stat[CRI] << " （クリ率" << p_max.cri_rate * 100 << "％、倍率" << p_max.cri_dmg * 100 << "％）" << endl
				<< "　DH   " << p_max.stat[DH] << " （DH率" << p_max.dh_rate * 100 << "％）" << endl
				<< "　意思 " << p_max.stat[DET] << endl
				<< "　SS   " << p_max.stat[SS] << " （" << "GCD " << p_max.gcd << "s、dot倍率" << p_max.dot_dmg * 100 << "％）" << endl
				<< "　信仰 " << p_max.stat[PIE] << endl
				<< "　（マテリア情報）武略ｱﾙﾃﾏx" << m1_nummax[0] << "、天眼ｱﾙﾃﾏx" << m1_nummax[1] << "、雄略ｱﾙﾃﾏx" << m1_nummax[2] << "、詠唱ｱﾙﾃﾏx" << m1_nummax[3] << "、信力ｱﾙﾃﾏx" << m1_nummax[4] << endl
				<< "　　　　　　　　　武略ｵﾒｶﾞx" << m2_nummax[0] << "、天眼ｵﾒｶﾞx" << m2_nummax[1] << "、雄略ｵﾒｶﾞx" << m2_nummax[2] << "、詠唱ｵﾒｶﾞx" << m2_nummax[3] << "、信力ｵﾒｶﾞx" << m2_nummax[4] << endl
				<< "　　　　　　　　　マテリアによるステータス：Cri+" << m_stmax[0] << " DH+" << m_stmax[1] << " Det+" << m_stmax[2] << " SS+" << m_stmax[3] << " Pie+" << m_stmax[4] << endl
				<< "【総合ダメージ倍率】" << p_max.sage_dmg_rate * 100 << "％" << endl;
		fout.close();
	}
	else { return 0; }


	return 0;
}