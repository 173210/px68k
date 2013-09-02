// JOYSTICK.C - ���祤���ƥ��å����ݡ��� for WinX68k
// DInput�ν��������λ�ȡ����祤���ƥ��å��ݡ��ȥ����å�

#include "common.h"
#include "prop.h"
#include "joystick.h"
#ifdef PSP
#include <pspctrl.h>
#endif
#ifdef ANDROID
#include <SDL.h>
#include <android/log.h>
#endif

#if 0
LPDIRECTINPUT		dinput = NULL;
#endif

#ifndef MAX_BUTTON
#define MAX_BUTTON 32
#endif

char joyname[2][MAX_PATH];
char joybtnname[2][MAX_BUTTON][MAX_PATH];
BYTE joybtnnum[2] = {0, 0};

//static	int		joyactive = 0;
BYTE joy[2];
BYTE JoyKeyState;
BYTE JoyKeyState0;
BYTE JoyKeyState1;
BYTE JoyState0[2];
BYTE JoyState1[2];
BYTE JoyPortData[2];

#define SET_VBTN(id, bx, by)				\
{							\
	vbtn_state[id] = VBTN_OFF;			\
	vbtn_rect[id].x = (float)bx;			\
	vbtn_rect[id].y = (float)by;			\
	vbtn_rect[id].x2 = (float)(bx) + VBTN_WIDTH;	\
	vbtn_rect[id].y2 = (float)(by) + VBTN_HEIGHT;	\
}

void Joystick_Init(void)
{
	joy[0] = 1;  // �Ȥꤢ��������ܤ���ͭ��
	joy[1] = 0;
	JoyKeyState = 0;
	JoyKeyState0 = 0;
	JoyKeyState1 = 0;
	JoyState0[0] = 0xff;
	JoyState0[1] = 0xff;
	JoyState1[0] = 0xff;
	JoyState1[1] = 0xff;
	JoyPortData[0] = 0;
	JoyPortData[1] = 0;
#ifdef ANDROID
	int i;

	for (i = 0; i < VBTN_MAX; i++) {
		vbtn_state[i] = VBTN_NOUSE;
	}

	//�ƥ������������Ȥ��ä���ˤ��������Ȥꤢ�����̡�������
	//�ܥ����Texture id��1 origin���ܥ���id��0 origin�ʤΤ����
	// �����岼 (��岼����������BA�ǤϤʤ�)
	SET_VBTN(0, 20, 450);
	SET_VBTN(1, 100, 450);
	SET_VBTN(2, 60, 400);
	SET_VBTN(3, 60, 500); 

	// �ܥ���
	SET_VBTN(4, 680, 450);
	SET_VBTN(5, 750, 450); 
#endif
}

void Joystick_Cleanup(void)
{
#if 0
	if (joy[0]) IDirectInputDevice2_Release(joy[0]);
	if (joy[1]) IDirectInputDevice2_Release(joy[1]);
	if (dinput) IDirectInput_Release(dinput);
#endif
}


void Joystick_Activate(UINT wParam)
{
#if 1
	(void)wParam;
#else
	if (wParam != WA_INACTIVE)
	{
		if (joy[0]) IDirectInputDevice2_Acquire(joy[0]);
		if (joy[1]) IDirectInputDevice2_Acquire(joy[1]);
	}
	else
	{
		if (joy[0]) IDirectInputDevice2_Unacquire(joy[0]);
		if (joy[1]) IDirectInputDevice2_Unacquire(joy[1]);
	}
#endif
}


BYTE FASTCALL Joystick_Read(BYTE num)
{
	BYTE joynum = num;
	BYTE ret0 = 0xff, ret1 = 0xff, ret;

	(void)joynum;
	(void)ret0;
	(void)ret1;
	ret = 0xff;

	Config.JoyKey = 1;

	if (Config.JoySwap) joynum ^= 1;
	if (joy[num]) {
		ret0 = JoyState0[num];
		ret1 = JoyState1[num];
	}

	if (Config.JoyKey)
	{
		if ((Config.JoyKeyJoy2)&&(num==1))
			ret0 ^= JoyKeyState;
		else if ((!Config.JoyKeyJoy2)&&(num==0))
			ret0 ^= JoyKeyState;
	}

	ret = ((~JoyPortData[num])&ret0)|(JoyPortData[num]&ret1);

	if (ret0 != 0xff || ret != 0xff)
		printf("ret0: 0x%x, ret: 0x%x\n", ret0, ret);

	return ret;
}


void FASTCALL Joystick_Write(BYTE num, BYTE data)
{
	if ( (num==0)||(num==1) ) JoyPortData[num] = data;
}

void FASTCALL Joystick_Update(void)
{
#if defined(PSP)
	BYTE ret0 = 0xff, ret1 = 0xff;
	int num = 0; //xxx �Ȥꤢ����JOY1�Τߡ�

	SceCtrlData psppad;
	sceCtrlPeekBufferPositive(&psppad, 1);

	if (psppad.Buttons & PSP_CTRL_LEFT) {
		ret0 ^= JOY_LEFT;
	}
	if (psppad.Buttons & PSP_CTRL_RIGHT) {
		ret0 ^= JOY_RIGHT;
	}
	if (psppad.Buttons & PSP_CTRL_UP) {
		ret0 ^= JOY_UP;
	}
	if (psppad.Buttons & PSP_CTRL_DOWN) {
		ret0 ^= JOY_DOWN;
	}
	if (psppad.Buttons & PSP_CTRL_CIRCLE) {
		ret0 ^= JOY_TRG1;
	}
	if (psppad.Buttons & PSP_CTRL_CROSS) {
		ret0 ^= JOY_TRG2;
	}

	JoyState0[num] = ret0;
	JoyState1[num] = ret1;
#elif defined(ANDROID)
	SDL_Finger *finger;
	static SDL_TouchID touchId = -1;
	SDL_FingerID fid;
	float fx, fy;
	int i, j;

	// �����touchId����ˡ�Ǥ褤�Τ������
	if (touchId == -1) {
		// �Ȥꤢ����10�Ĥ��餤�����å�����Ф������ʡ�
		for (i = 0; i < 10; i++) {
			// �Ȥꤢ������3��ʬ���餤�����å�
			for (j = 0; j < 3; j++) {
				if (SDL_GetTouchFinger(i, j) != NULL) {
					touchId = i;
					__android_log_print(ANDROID_LOG_DEBUG,"Tag","id: %d x: %f y: %f", i, fx, fy);
					goto done;
				}
			}
		}
	}
done:
	if (touchId == -1)
		return;

	// �������ʪ�����ƥ��դ˥ꥻ�åȤ���
	for (i = 0; i < VBTN_MAX; i++) {
		if (vbtn_state[i] != VBTN_NOUSE) {
			vbtn_state[i] = VBTN_OFF;
		}
	}

	// ���νֲִ�����Ƥ���ܥ�������򥪥�ˤ���
	for (i = 0; i < FINGER_MAX; i++) {
		finger = SDL_GetTouchFinger(touchId, i);
		if (!finger)
			continue;

		fx = finger->x;
		fy = finger->y;

		//__android_log_print(ANDROID_LOG_DEBUG,"Tag","id: %d x: %f y: %f", i, fx, fy);

		for (j = 0; j < VBTN_MAX; j++) {
			if (vbtn_state[j] == VBTN_NOUSE)
				continue;
			// ��ǽ��ͤ���Ĥ���Ƚ��
			if (vbtn_rect[j].x / 800.0 > fx)
				continue;
			if (vbtn_rect[j].x2 / 800.0 < fx)
				continue;
			if (vbtn_rect[j].y / 600.0 > fy)
				continue;
			if (vbtn_rect[j].y2 / 600.0 < fy)
				continue;

			//�ޥå������饪��ˤ���
			vbtn_state[j] = VBTN_ON;
			//���ۥܥ���ϽŤʤ�ʤ�
			break;
		}
	}

	BYTE ret0 = 0xff, ret1 = 0xff;
	int num = 0; //xxx �Ȥꤢ����JOY1�Τߡ�

	if (vbtn_state[0] == VBTN_ON) {
		ret0 ^= JOY_LEFT;
	}
	if (vbtn_state[1] == VBTN_ON) {
		ret0 ^= JOY_RIGHT;
	}
	if (vbtn_state[2] == VBTN_ON) {
		ret0 ^= JOY_UP;
	}
	if (vbtn_state[3] == VBTN_ON) {
		ret0 ^= JOY_DOWN;
	}
	if (vbtn_state[4] == VBTN_ON) {
		ret0 ^= JOY_TRG1;
	}
	if (vbtn_state[5] == VBTN_ON) {
		ret0 ^= JOY_TRG2;
	}

	JoyState0[num] = ret0;
	JoyState1[num] = ret1;
#endif
}

