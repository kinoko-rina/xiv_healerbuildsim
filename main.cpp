#include <iostream>
#include <fstream>
#include <math.h>
#include <string>

using namespace std;

#define MATERIA1_VALUE 36	//��ʃ}�e���A�̐��l�i�A���e�}�e���W��+36�j
#define MATERIA2_VALUE 12	//���ʃ}�e���A�̐��l�i�I���K�}�e���W��+12�j

// �e�X�e�[�^�X�̏����l
#define INIT_CRI 400
#define INIT_DH 400
#define INIT_DET 390
#define INIT_SS 400
#define INIT_PIE 390

// Player,Equipment��stat�ɑΉ�
#define CRI 0
#define DH 1
#define DET 2
#define SS 3
#define PIE 4

// Equipment��area�ɑΉ�
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



// �v���C���[
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

	//����180�b�ł̃_���[�W���Ғl�̌v�Z�l�idot��؂�ڂȂ��X�V�j
	double calc_sage_damage_part(double t_time, double t_dot_dmg,double t_gcd, double t_eukrasia_num){
		return (70.0/3.0 * t_dot_dmg) * t_time +	//�G�E�N���V�A�E�h�V�XIII (dot)
			330.0 * ((t_time - 2.5 * ceil(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*ceil(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//�h�V�XIII
			510.0 * (t_time / 45.0);	//�t���O�}
	}

	//����180�b�ł̃_���[�W���Ғl�̌v�Z�l�idot��؂炵�čX�V�j
	double calc_sage_damage_part2(double t_time, double t_dot_dmg, double t_gcd, double t_eukrasia_num){
		return (70.0 / 3.0 * t_dot_dmg) * floor(t_time / 30.0) * 30.0 +	//�G�E�N���V�A�E�h�V�XIII (dot)
			330.0 * ((t_time - 2.5 * floor(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*floor(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//�h�V�XIII
			510.0 * (t_time / 45.0);	//�t���O�}
	}

	//����180�b�ł̃_���[�W���Ғl�̌v�Z�l�iMIN�l��dot�X�V�̃��X���ő�j
	double calc_sage_damage_part3min(double t_time, double t_dot_dmg, double t_gcd, double t_eukrasia_num){
		return (70.0 / 3.0 * t_dot_dmg) * floor(t_time / 30.0) * 28.75 +	//�G�E�N���V�A�E�h�V�XIII (dot)
			330.0 * ((t_time - 2.5 * floor(t_time / 30.0) - t_eukrasia_num * (2.5 - t_gcd)*floor(t_time / 30.0)) / t_gcd - (t_time / 45.0)) +	//�h�V�XIII
			510.0 * (t_time / 45.0);	//�t���O�}
	}


	//����180�b�ł̃_���[�W���Ғl�i�{���j
	double calc_sage_damage(){
		const double eukrasia_num = 0.5; //30�b�P�ʂŁAdot�t�^�ȊO�ŃG�E�N���V�A���g����
		const double basedamage = calc_sage_damage_part(180.0, 1.0, 2.5, eukrasia_num) * 1.02;

		/* 
		//dot�X�V�^�C�~���O�܂Ŋ܂߁A�ڍׂɌv�Z����
		double calc_time1 = 180.0 - ((27.5 - 2.5 * eukrasia_num) - gcd * floor((27.5 - 2.5 * eukrasia_num) / gcd)) * 6.0;
		double t_sage_dmg1 = calc_sage_damage_part(calc_time1, dot_dmg, gcd, eukrasia_num);
		double calc_time2 = 180.0 - ((27.5 - 2.5 * eukrasia_num) - gcd * ceil((27.5 - 2.5 * eukrasia_num) / gcd)) * 6.0;
		double t_sage_dmg2 = calc_sage_damage_part2(calc_time2, dot_dmg, gcd, eukrasia_num);
		double result1 = (t_sage_dmg1* cri_avg * dh_avg * det_avg) / (basedamage * calc_time1 / 180.0);
		double result2 = (t_sage_dmg2* cri_avg * dh_avg * det_avg) / (basedamage * calc_time2 / 180.0);
		if (result1 > result2){ sage_dmg = t_sage_dmg1; return result1; }
		else{ sage_dmg = t_sage_dmg2; return result2; }
		*/

		//dot���X���l�������ő�p�^�[���ƍŏ��p�^�[���𑫂���2�Ŋ���v�Z���@
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

// �}�e���A�����f�[�^
struct MateriaData{
	int slotinfo[5]; // 5����}�e���A���̏�� 0���Ȃ��@1��ʃ}�e���A���@2���ʃ}�e���A��
	int statmax[5]; // �T�u�X�e���Ƃ̃T�u�X�e����l

	int slot[5]; // �}�e���A���ɂǂ̃}�e���A���h�����Ă��邩 CRI0 DH1 DET2 SS3 PIE4
	int stat[5]; // �T�u�X�e���ƂɎ��ۂɏ㏸����X�e�[�^�X

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

// ����
struct Equipment{
	string name; // ������
	int enable;
	int area; // ��������
	int stat[5]; // �����ɂ��Ă���T�u�X�e�̒l
	int statmax; // �T�u�X�e����l
	int materia1_slotnum; // ��ʃ}�e���A���̐�
	int materia2_slotnum; // ���ʃ}�e���A���̐�
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

// �H��
struct Food{
	string name; // �H����
	int enable;
	double stat_p[5]; // �X�e�[�^�X�{��
	int stat_max[5]; // �X�e�[�^�X�̏���l
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


// �}�e���A�����ۂɑ����\�����`�F�b�N����
int MateriaCheck(MateriaData (&m)[11], int m1_num[5], int m2_num[5]){

	int check = 0; // �߂�l�p�B����l�I�[�o�[�▢�������������ꍇ�Acheck=1�ɕς���

	// ��ʃ}�e���A�̃`�F�b�N�B���v�������Ԃ񃋁[�v����
	int all_m1_num = m1_num[0] + m1_num[1] + m1_num[2] + m1_num[3] + m1_num[4]; //��ʃ}�e���A�𑕒����鍇�v��
	for (int temp = 0; temp < all_m1_num; temp++){

		// ��������ǂ̃T�u�X�e�̃}�e���A���͂߂邩���肷��(�������Ƒ����\�ꏊ�̍�(dif)���������T�u�X�e��D�悷��)
		int dif_min = 99;
		int now_st = -1;
		for (int st = 0; st < 5; st++){
			if (m1_num[st] == 0){ continue; } // �}�e���A�����\�萔0�̏ꍇ�͎��̃T�u�X�e�փX�L�b�v
			int t_n = 0; // ���̃}�e���A�������ł���ő吔
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
				//cout << "(M1)stat[" << st << "]x" << m1_num[st] << "�� �󂫂��Ȃ������ł���"<<endl;
				check = 1;
				continue;
			} // �}�e���A�����ł���ꏊ��0�̏ꍇ�͎��̃T�u�X�e�փX�L�b�v
			int dif = t_n - m1_num[st];
			//cout << temp << "_dif(" << st << ")=" << dif << endl;
			if (dif < dif_min){ dif_min = dif; now_st = st; }
		}
		if (now_st == -1){ break; } // ���łɂ͂߂���}�e���A�����i����I�[�o�[�j
		// �����܂ł�now_st������i�ǂ̃T�u�X�e�̃}�e���A������j


		// ��������ǂ�area�̌��Ƀ}�e���A���͂߂邩���肷��(����l(st_max)�ɍł��]�T�̂���area�̌��ɑ�������)
		int st_max = 0;
		int now_area = -1;
		for (int area = 0; area < 11; area++){
			if (st_max < m[area].statmax[now_st]){
				for (int i = 0; i < 5; i++){
					// ����area��1�ł��A�󂫂̂����ʃ}�e���A�������݂����ok
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 1){
						now_area = area;
						st_max = m[area].statmax[now_st];
						break;
					}
				}
			}
		}
		// �����܂ł�now_area������i�}�e���A�𑕒����镔�ʌ���j


		// ��������}�e���A�𑕒����鏈��
		for (int i = 0; i < 5; i++){
			// �󂫂̂����ʃ}�e���A��
			if (m[now_area].slot[i] == -1 && m[now_area].slotinfo[i] == 1){
				m[now_area].slot[i] = now_st;
				m1_num[now_st]--;
				if (m[now_area].statmax[now_st] < MATERIA1_VALUE){ // ����l�I�[�o�[�ő���
					//cout << "(M1)area" << now_area << "�̃X���b�g[" << i << "]�ɑ���->stat[" << now_st << "]+" << m[now_area].statmax[now_st] << "(over!)" << endl;
					m[now_area].stat[now_st] += m[now_area].statmax[now_st];
					m[now_area].statmax[now_st] = 0;
					check = 1;
				}
				else{ // ����ɑ���
					//cout << "(M1)area" <<  now_area << "�̃X���b�g[" << i << "]�ɑ���->stat[" << now_st << "] + " << MATERIA1_VALUE << endl;
					m[now_area].stat[now_st] += MATERIA1_VALUE;
					m[now_area].statmax[now_st] -= MATERIA1_VALUE;
				}
				break;
			}
		}
		// �����܂Ń}�e���A��������

	}
	// ��ʃ}�e���A�̏�������


	// ���ʃ}�e���A�̃`�F�b�N�B���v�������Ԃ񃋁[�v����
	int all_m2_num = m2_num[0] + m2_num[1] + m2_num[2] + m2_num[3] + m2_num[4]; //��ʃ}�e���A�𑕒����鍇�v��
	for (int temp = 0; temp < all_m2_num; temp++){

		// ��������ǂ̃T�u�X�e�̃}�e���A���͂߂邩���肷��(�������Ƒ����\�ꏊ�̍�(dif)���������T�u�X�e��D�悷��)
		int dif_min = 99;
		int now_st = -1;
		for (int st = 0; st < 5; st++){
			if (m2_num[st] == 0){ continue; } // �}�e���A�����\�萔0�̏ꍇ�͎��̃T�u�X�e�փX�L�b�v
			int t_n = 0; // ���̃}�e���A��������ő吔
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
				//cout << "(M2)stat[" << st << "]x" << m2_num[st] << "�� �󂫂��Ȃ������ł���"<< endl;
				check = 1;
				continue;
			} // �}�e���A�����ł���ꏊ��0�̏ꍇ�͎��̃T�u�X�e�փX�L�b�v
			int dif = t_n - m2_num[st];
			//cout << "dif2(" << st << ")=" << dif << endl;
			if (dif < dif_min){ dif_min = dif; now_st = st; }
		}
		if (now_st == -1){ break; } // ���łɂ͂߂���}�e���A�����i����I�[�o�[�j
		// �����܂ł�now_st������i�ǂ̃T�u�X�e�̃}�e���A������j


		// ��������ǂ�area�̌��Ƀ}�e���A���͂߂邩���肷��(����l(st_max)�ɍł��]�T�̂���area�̌��ɑ�������)
		int st_max = 0;
		int now_area = -1;
		for (int area = 0; area < 11; area++){
			if (st_max < m[area].statmax[now_st]){
				for (int i = 0; i < 5; i++){
					// ����area��1�ł��A�󂫂̂��鉺�ʃ}�e���A�������݂����ok
					if (m[area].slot[i] == -1 && m[area].slotinfo[i] == 2){
						now_area = area;
						st_max = m[area].statmax[now_st];
						break;
					}
				}
			}
		}
		// �����܂ł�now_area������i�}�e���A�𑕒����镔�ʌ���j


		// ��������}�e���A�𑕒����鏈��
		for (int i = 0; i < 5; i++){
			// �󂫂̂��鉺�ʃ}�e���A��
			if (m[now_area].slot[i] == -1 && m[now_area].slotinfo[i] == 2){
				m[now_area].slot[i] = now_st;
				m2_num[now_st]--;
				if (m[now_area].statmax[now_st] < MATERIA2_VALUE){ // ����l�I�[�o�[�ő���
					//cout << "(M2)area" << now_area << "�̃X���b�g[" << i << "]�ɑ���->stat[" << now_st << "]+" << m[now_area].statmax[now_st] << "(over!)" << endl;
					m[now_area].stat[now_st] += m[now_area].statmax[now_st];
					m[now_area].statmax[now_st] = 0;
					check = 1;
				}
				else{ // ����ɑ���
					//cout << "(M2)area" <<  now_area << "�̃X���b�g[" << i << "]�ɑ���->stat[" << now_st << "] + " << MATERIA2_VALUE << endl;
					m[now_area].stat[now_st] += MATERIA2_VALUE;
					m[now_area].statmax[now_st] -= MATERIA2_VALUE;
				}
				break;
			}
		}
		// �����܂Ń}�e���A��������

	}
	// ���ʃ}�e���A�̏�������

	return check;
}

int main(){

	cin.exceptions(ios::failbit);

	Equipment equip[11][4]; // ����
	Food food[5];

	// �����t�@�C���̓ǂݍ���
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

	// �H���t�@�C���̓ǂݍ���
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
	//�T�u�X�e���Ԃ�0.Cri 1.DH 2.Det 3.SS 4.Pie
	equip[0][0].set("����|�V��", WEAPON, 0, 0, 0, 253, 177, 3, 2, 253);
	equip[0][1].set("����|�Ɂ@", WEAPON, 177, 0, 253, 0, 0, 2, 0, 253);
	equip[1][0].set("���@|�V��", HEAD, 0, 0, 108, 154, 0, 3, 2, 154);
	//equip[1][1].set("���@|N �@", HEAD, 0, 0, 108, 154, 0, 2, 0, 154);
	equip[2][0].set("���@|�V��", BODY, 0, 0, 244, 171, 0, 3, 2, 244);
	equip[2][1].set("���@|N �@", BODY, 244, 0, 171, 0, 0, 2, 0, 244);
	equip[3][0].set("��@|�V��", HANDS, 154, 108, 0, 0, 0, 3, 2, 154);
	//equip[3][1].set("��@|N �@", HANDS, 0, 0, 154, 0, 108, 2, 0, 154);
	equip[4][0].set("�r�@|�V��", LEGS, 0, 0, 0, 244, 171, 3, 2, 244);
	equip[4][1].set("�r�@|N �@", LEGS, 244, 0, 171, 0, 0, 2, 0, 244);
	equip[5][0].set("���@|�V��", FEET, 108, 0, 154, 0, 0, 3, 2, 154);
	//equip[5][1].set("���@|N �@", FEET, 0, 0, 154, 108, 0, 2, 0, 154);
	equip[6][0].set("���@|�V��", EARINGS, 121, 0, 0, 0, 85, 2, 3, 121);
	equip[7][0].set("��@|�V��", NECKLACE, 0, 0, 121, 85, 0, 2, 3, 121);
	equip[8][0].set("�r�@|�V��", BRACELETS, 0, 0, 85, 0, 121, 2, 3, 121);
	equip[9][0].set("�w1 |�V��", LRING, 121, 0, 0, 0, 85, 2, 3, 121);
	equip[10][0].set("�w2 |�V��", RRING, 0, 0, 85, 121, 0, 2, 3, 121);

	//food[0].set("�Ȃ�", 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
	food[0].set("�R�R�i�b�c�R�b�h�`���E�_�[HQ", 0.1, 0.0, 0.1, 0.0, 0.0, 46, 0, 76, 0, 0);
	food[1].set("�o�o���A�E�I�E�V���[�R��HQ", 0.0, 0.1, 0.1, 0.0, 0.0, 0, 82, 50, 0, 0);
	*/

	Player p_init;

	int m1_slotnum = 0; // ��ʃ}�e���A���Ƃ錊�̐�
	int m2_slotnum = 0; // ���ʃ}�e���A���Ƃ錊�̐�
	int m1_maxnum[5] = { 0, 0, 0, 0, 0 }; // ����l���l�������A�X�e�[�^�X���Ƃ̏�ʃ}�e���A���Ƃ錊�̐�
	int m2_maxnum[5] = { 0, 0, 0, 0, 0 }; // ����l���l�������A�X�e�[�^�X���Ƃ̉��ʃ}�e���A���Ƃ錊�̐�
	MateriaData equip_m_init[11]; // ���ʂ��Ƃ̃}�e���A��� �����l
	int selected_equip[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //���ʂ��ƂɑI�񂾑���

	// �ő�ƂȂ錋�ʂ�ۑ����邽�߂̕ϐ��Q
	Player p_max;
	MateriaData equip_m_max[11];
	int food_maxnum = 0;
	double d_max = 0.0;
	int selected_equip_max[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


	// �����̃p�^�[�������v�Z�i��̃��[�v�Ŏg�p�j
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
	

	// ���[�U�[���͍��ڂ�������

	// �M�̕K�v�l�@����ȏ�̐M���m�ۂ���悤�ɂ���B0�̏ꍇ�A�M�}�e���A��0�ɂ��邽�ߏ����������Ȃ�
	int set_min_pie = 0;
	cout << "�M��(Pie)�̕K�v�l����́B���͒l�ȏ�̐M���m�ۂ���悤�Ƀr���h��g�݂܂��B" << endl;
	while (true){
		try{ cin >> set_min_pie; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (set_min_pie < 0) { set_min_pie = 0; }

	// gcd�̍ŏ��l�@����ȏ��gcd�ɂȂ�悤�ɂ���i��������gcd��h�~���邽�߂̃I�v�V�����j0.0�̏ꍇ�͐����Ȃ�
	double set_gcd_limit = 0.0;
	cout << "GCD�̍ŏ��l�����(0.0-2.5)�B���͒l�ȏ��GCD�ƂȂ�悤�Ƀr���h��g�݂܂��B" << endl << "�i��������GCD��h�~�������Ƃ��Ɏg�p���Ă��������j" << endl;
	while (true){
		try{ cin >> set_gcd_limit; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (set_gcd_limit > 2.5) { set_min_pie = 2.5; }
	else if (set_gcd_limit < 0.0) { set_min_pie = 0.0; }

	// ���[�U�[���͍��ڂ����܂�


	// loop�J�n
	for (int loop = 0; loop < max_equip_pattern; loop++){

		//system("cls");
		cout << endl << loop + 1 << "/" << max_equip_pattern << " �v�Z��...";

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

	// �����ɑ����\�ȃ}�e���A�̌��ȂǁA�������̏����B�������ʂ��Ƃɏ������s��
	for (int area = 0; area < 11; area++){

		m1_slotnum += equip[area][selected_equip[area]].materia1_slotnum;
		m2_slotnum += equip[area][selected_equip[area]].materia2_slotnum;

		// �}�e���A�̃X���b�g������
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

	// ��x�v�Z
	p_init.calc_stat();

	// �f�̃X�e�[�^�X�Ŋ���GCD�����~�b�g�𒴂��Ă����ꍇ�ANG
	if (set_gcd_limit > p_init.gcd){ cout << "(skip)"; continue; }

	int check_pie = 1; // loop����pie�������`�F�b�N���邩�ǂ���(1�Ń`�F�b�N����)
	// PIE�������Ȃ��ꍇ�APIE�}�e���A�̌���0�ɐݒ肵�Ă���
	if (set_min_pie == 0) {
		m1_maxnum[4] = 0;
		m2_maxnum[4] = 0;
		check_pie = 0;
	}
	// �f�̃X�e�[�^�X�Ŋ��ɕK�vPIE�𒴂��Ă����ꍇ���APIE�}�e���A�̌���0�ɐݒ�
	else if (set_min_pie < p_init.stat[PIE]){
		m1_maxnum[4] = 0;
		m2_maxnum[4] = 0;
		check_pie = 0;
	}


	// ���ׂĂ̏�ʃ}�e���A���p�^�[�����V�~�����[�g����
	int s1[5] = { 0, 0, 0, 0, 0 }; // �X�e�[�^�X���Ƃ̏�ʃ}�e���A�̑�����
	for (s1[0] = 0; s1[0] <= m1_slotnum && s1[0] <= m1_maxnum[0]; s1[0]++){
		for (s1[1] = 0; s1[1] <= m1_slotnum - s1[0] && s1[1] <= m1_maxnum[1]; s1[1]++){
			for (s1[2] = 0; s1[2] <= m1_slotnum - s1[0] - s1[1] && s1[2] <= m1_maxnum[2]; s1[2]++){
				for (s1[3] = 0; s1[3] <= m1_slotnum - s1[0] - s1[1] - s1[2] && s1[3] <= m1_maxnum[3]; s1[3]++){
					s1[4] = m1_slotnum - s1[0] - s1[1] - s1[2] - s1[3];
					if (s1[4] > m1_maxnum[4]) { continue; }

					// ���ׂẲ��ʃ}�e���A���p�^�[�����V�~�����[�g����
					int s2[5] = { 0, 0, 0, 0, 0 }; // �X�e�[�^�X���Ƃ̉��ʃ}�e���A�̑�����
					for (s2[0] = 0; s2[0] <= m2_slotnum && s2[0] <= m2_maxnum[0]; s2[0]++){
						for (s2[1] = 0; s2[1] <= m2_slotnum - s2[0] && s2[1] <= m2_maxnum[1]; s2[1]++){
							for (s2[2] = 0; s2[2] <= m2_slotnum - s2[0] - s2[1] && s2[2] <= m2_maxnum[2]; s2[2]++){
								for (s2[3] = 0; s2[3] <= m2_slotnum - s2[0] - s2[1] - s2[2] && s2[3] <= m2_maxnum[3]; s2[3]++){
									s2[4] = m2_slotnum - s2[0] - s2[1] - s2[2] - s2[3];
									if (s2[4] > m2_maxnum[4]) { continue; }

									for (int f = 0; f < 5; f++){ // �œK�ȐH����I��
										if (food[f].enable == false){ continue; }

										Player p = p_init;
										for (int st = 0; st < 5; st++){
											p.stat[st] += s1[st] * MATERIA1_VALUE + s2[st] * MATERIA2_VALUE; // �}�e���A���̃T�u�X�e���Z
											int food_plus = p.stat[st] * food[f].stat_p[st]; // �H�����̑����v�Z
											if (food_plus>food[f].stat_max[st]){ food_plus = food[f].stat_max[st]; }
											p.stat[st] += food_plus;
										}
										if (check_pie == 1) {
											if (set_min_pie > p.stat[PIE]){ continue; } // �M���K�v�l�ɖ����Ȃ������ꍇ��NG
											if (set_min_pie + MATERIA1_VALUE < p.stat[PIE]){ if (s1[4]>0 || s2[4] > 0) { continue; } } // �}�e���A���g�p���Ă���ꍇ�A�M���K�v�l���傫���I�[�o�[���Ă���ꍇ��NG
										}
										p.calc_stat(); // ���Ғl�v�Z
										if (p.sage_dmg_rate > d_max){ // �ő�l�X�V��

											// �}�e���A�������\���`�F�b�N
											MateriaData equip_m[11];
											for (int i = 0; i < 11; i++){ equip_m[i] = equip_m_init[i]; }
											int m1_num[5] = { 0, 0, 0, 0, 0 };
											int m2_num[5] = { 0, 0, 0, 0, 0 };
											for (int i = 0; i < 5; i++){
												m1_num[i] = s1[i];
												m2_num[i] = s2[i];
											}
											if (MateriaCheck(equip_m, m1_num, m2_num) == 0){ // �}�e���A������ok
												if (set_gcd_limit > p.gcd){ continue; } // gcd�����~�b�g�𒴂��Ă��܂����ꍇ��NG
												d_max = p.sage_dmg_rate;
												p_max = p;
												food_maxnum = f;
												for (int i = 0; i < 11; i++){ equip_m_max[i] = equip_m[i]; selected_equip_max[i] = selected_equip[i]; }
												//cout << s1[0] << "," << s1[1] << "," << s1[2] << "," << s1[3] << "," << s1[4] << "-" << s2[0] << "," << s2[1] << "," << s2[2] << "," << s2[3] << "," << s2[4] << "-> �{��:" << p.sage_dmg_rate * 100 << "��" << endl;
											}
											else{ //�}�e���A�����ɏ���I�[�o�[or���������������ꍇ�A��x�����l�ɖ߂�����}�e���A���𑝉������Čv�Z
												p = p_init;
												for (int s = 0; s < 5; s++){
													for (int a = 0; a < 11; a++){
														p.stat[s] += equip_m[a].stat[s];
													}
													int food_plus = p.stat[s] * food[f].stat_p[s]; // �H�����̑����v�Z
													if (food_plus>food[f].stat_max[s]){ food_plus = food[f].stat_max[s]; }
													p.stat[s] += food_plus;
												}
												if (set_min_pie > p.stat[PIE]){ continue; } // �M���K�v�l�ɖ����Ȃ������ꍇ��NG
												p.calc_stat(); // ���߂Ċ��Ғl�v�Z
												if (p.sage_dmg_rate > d_max){ // ����ł��ő�l���X�V���ꂽ�ꍇ
													if (set_gcd_limit > p.gcd){ continue; } // gcd�����~�b�g�𒴂��Ă��܂����ꍇ��NG
													d_max = p.sage_dmg_rate;
													p_max = p;
													food_maxnum = f;
													for (int i = 0; i < 11; i++){ equip_m_max[i] = equip_m[i]; selected_equip_max[i] = selected_equip[i]; }
													//cout << s1[0] << "," << s1[1] << "," << s1[2] << "," << s1[3] << "," << s1[4] << "-" << s2[0] << "," << s2[1] << "," << s2[2] << "," << s2[3] << "," << s2[4] << "-> �{��:" << p.sage_dmg_rate * 100 << "�� *�}�e���A����I�[�o�[�ōX�V" << endl;
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
	// loop�I��

	// ���ʕ\���̏���
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

	// ���ʂ̕\��
	cout << endl << endl << "====���ʕ\��====" << endl;
	if (set_min_pie>0){ cout << "�ǉ������F�M��" << set_min_pie << " �ȏ�" << endl; }
	if (set_gcd_limit>0){ cout << "�ǉ������FGCD" << set_gcd_limit << "s �ȏ�" << endl; }
	cout << "�y�����ڍׁz" << endl;
	for (int i = 0; i < 11; i++){
		cout << "�@" << equip[i][selected_equip_max[i]].name << "|";
		for (int s = 0; s < 5; s++){
			if (equip_m_max[i].slotinfo[s] != -1){ 
				switch (equip_m_max[i].slot[s]){
					case 0: cout << "����"; break;
					case 1: cout << "�V��"; break;
					case 2: cout << "�Y��"; break;
					case 3: cout << "�r��"; break;
					case 4: cout << "�M��"; break;
				}
				switch (equip_m_max[i].slotinfo[s]){
					case 1: cout << "���ρ@"; break;
					case 2: cout << "�Ҷށ@"; break;
				}
			}
		}
		cout << endl;
	}
	cout << "�y�H���z" << food[food_maxnum].name << endl;
	cout << "�y�X�e�[�^�X�z" << endl << "�@Cri  " << p_max.stat[CRI] << " �i�N����" << p_max.cri_rate * 100 << "���A�{��" << p_max.cri_dmg * 100 << "���j" << endl
		<< "�@DH   " << p_max.stat[DH] << " �iDH��" << p_max.dh_rate * 100 << "���j" << endl
		<< "�@�ӎv " << p_max.stat[DET] << endl
		<< "�@SS   " << p_max.stat[SS] << " �i" << "GCD " << p_max.gcd << "s�Adot�{��" << p_max.dot_dmg * 100 << "���j" << endl
		<< "�@�M�� " << p_max.stat[PIE] << endl
		<< "�@�i�}�e���A���j��������x" << m1_nummax[0] << "�A�V�����x" << m1_nummax[1] << "�A�Y������x" << m1_nummax[2] << "�A�r������x" << m1_nummax[3] << "�A�M�ͱ���x" << m1_nummax[4] << endl
		<< "�@�@�@�@�@�@�@�@�@�����Ҷ�x" << m2_nummax[0] << "�A�V��Ҷ�x" << m2_nummax[1] << "�A�Y���Ҷ�x" << m2_nummax[2] << "�A�r���Ҷ�x" << m2_nummax[3] << "�A�M�͵Ҷ�x" << m2_nummax[4] << endl
		<< "�@�@�@�@�@�@�@�@�@�}�e���A�ɂ��X�e�[�^�X�FCri+" << m_stmax[0] << " DH+" << m_stmax[1] << " Det+" << m_stmax[2] << " SS+" << m_stmax[3] << " Pie+" << m_stmax[4] << endl
		<< "�y�����_���[�W�{���z" << p_max.sage_dmg_rate * 100 << "��" << endl;


	int if_out = 0;
	cout << endl  << "output.txt�֌��ʏo�͂��s���܂����H(1�ŏo�͂��s���܂�)" << endl;
	while (true){
		try{ cin >> if_out; }
		catch (...){ cin.clear(); cin.seekg(0); continue; }
		break;
	}
	if (if_out == 1) {
		// �t�@�C���o�͂��s��
		ofstream fout;
		fout.open("output.txt");
			fout << "====���ʕ\��====" << endl;
			if (set_min_pie>0){ fout << "�ǉ������F�M��" << set_min_pie << " �ȏ�" << endl; }
			if (set_gcd_limit>0){ fout << "�ǉ������FGCD" << set_gcd_limit << "s �ȏ�" << endl; }
			fout << "�y�����ڍׁz" << endl;
			for (int i = 0; i < 11; i++){
				fout << "�@" << equip[i][selected_equip_max[i]].name << "|";
				for (int s = 0; s < 5; s++){
					if (equip_m_max[i].slotinfo[s] != -1){ 
						switch (equip_m_max[i].slot[s]){
							case 0: fout << "����"; break;
							case 1: fout << "�V��"; break;
							case 2: fout << "�Y��"; break;
							case 3: fout << "�r��"; break;
							case 4: fout << "�M��"; break;
						}
						switch (equip_m_max[i].slotinfo[s]){
							case 1: fout << "���ρ@"; break;
							case 2: fout << "�Ҷށ@"; break;
						}
					}
				}
				fout << endl;
			}
			fout << "�y�H���z" << food[food_maxnum].name << endl;
			fout << "�y�X�e�[�^�X�z" << endl << "�@Cri  " << p_max.stat[CRI] << " �i�N����" << p_max.cri_rate * 100 << "���A�{��" << p_max.cri_dmg * 100 << "���j" << endl
				<< "�@DH   " << p_max.stat[DH] << " �iDH��" << p_max.dh_rate * 100 << "���j" << endl
				<< "�@�ӎv " << p_max.stat[DET] << endl
				<< "�@SS   " << p_max.stat[SS] << " �i" << "GCD " << p_max.gcd << "s�Adot�{��" << p_max.dot_dmg * 100 << "���j" << endl
				<< "�@�M�� " << p_max.stat[PIE] << endl
				<< "�@�i�}�e���A���j��������x" << m1_nummax[0] << "�A�V�����x" << m1_nummax[1] << "�A�Y������x" << m1_nummax[2] << "�A�r������x" << m1_nummax[3] << "�A�M�ͱ���x" << m1_nummax[4] << endl
				<< "�@�@�@�@�@�@�@�@�@�����Ҷ�x" << m2_nummax[0] << "�A�V��Ҷ�x" << m2_nummax[1] << "�A�Y���Ҷ�x" << m2_nummax[2] << "�A�r���Ҷ�x" << m2_nummax[3] << "�A�M�͵Ҷ�x" << m2_nummax[4] << endl
				<< "�@�@�@�@�@�@�@�@�@�}�e���A�ɂ��X�e�[�^�X�FCri+" << m_stmax[0] << " DH+" << m_stmax[1] << " Det+" << m_stmax[2] << " SS+" << m_stmax[3] << " Pie+" << m_stmax[4] << endl
				<< "�y�����_���[�W�{���z" << p_max.sage_dmg_rate * 100 << "��" << endl;
		fout.close();
	}
	else { return 0; }


	return 0;
}