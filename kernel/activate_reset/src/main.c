
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/rtc.h>
#include <psp2kern/syscon.h>
#include <taihen.h>


typedef struct SceNVSKitActivationData { // size is 0x20 bytes
	char magic[4]; // "act\n"
	SceUInt32 issue_no;
	SceUInt32 end_date;
	SceUInt32 start_date;
	char cmac_hash[0x10];
} SceNVSKitActivationData;

int module_get_export_func(SceUID pid, const char *modname, SceNID libnid, SceNID funcnid, uintptr_t *func);


int set_tool_secure_tick(SceUInt64 new_tick){

	SceUInt64 startDate0 = (new_tick * 1000 * 1000) + 0xDCBFFEFF2BC000LL; // unixtime to sce tick

	SceUInt32 tick_lo, tick_hi;
	char x[5];

	tick_lo = (uint32_t)(startDate0);
	tick_hi = (uint32_t)(startDate0 >> 0x20);

	x[0] = ((tick_lo >> 0x13) | (tick_hi << 0xD)) & 0xFF;
	x[1] = ((tick_lo >> 0x1B) | (tick_hi << 0x5)) & 0xFF;
	x[2] = (tick_hi >> 3) & 0xFF;
	x[3] = (tick_hi >> 0xB) & 0xFF;
	x[4] = (tick_hi >> 0x13) & 0xFF;
	int res = ksceSysconSendCommand(0x22, x, 5);
	if(res < 0){
		ksceKernelPrintf("sceSysconWriteScratchPadForDriver 0x%X", res);
		return res;
	}

	return 0;
}

int program_main(void){

	int res;
	int (* sceRtcGetCurrentToolSecureTick)(SceRtcTick *pTick);
	SceNVSKitActivationData nvs_act_data;
	SceRtcTick tick;
	SceUInt32 unixtime[2];
	SceDateTime date_time;

	module_get_export_func(0x10005, "SceRtc", 0x0351D827, 0xA0D7899A, (uintptr_t *)&sceRtcGetCurrentToolSecureTick);

	res = ksceSblNvsReadData(0x520, &nvs_act_data, sizeof(nvs_act_data));
	if(res < 0){
		ksceKernelPrintf("sceSblNvsReadData failed 0x%X\n", res);
		return res;
	}

	res = sceRtcGetCurrentToolSecureTick(&tick);
	if(res < 0){
		return res;
	}

	res = ksceRtcConvertTickToDateTime(&date_time, &tick);
	if(res < 0){
		return res;
	}

	res = ksceRtcConvertDateTimeToUnixTime(&date_time, unixtime);
	if(res < 0){
		return res;
	}

	if(unixtime[0] >= nvs_act_data.end_date){
		ksceKernelPrintf("Activation has already expired\n");
		ksceKernelPrintf("%31s %d %d\n", "Activation date", nvs_act_data.start_date, nvs_act_data.end_date);
		// ksceKernelPrintf("%31s 0x%llX\n", "ToolSecureTick", tick.tick);
		ksceKernelPrintf("%31s %04d/%02d/%02d %02d:%02d:%02d (%d %d)\n",
			"current ToolSecureTick",
			date_time.year, date_time.month, date_time.day, date_time.hour, date_time.minute, date_time.second,
			unixtime[0], unixtime[1]
		);

		set_tool_secure_tick((SceUInt64)nvs_act_data.start_date);
		ksceKernelPrintf("Doen setting activation start_date to ToolSecureTick\n");
	}else if((nvs_act_data.end_date - unixtime[0]) <= (60 * 60 * 24 * 10)){ // expire in 10-days

		ksceKernelPrintf("%31s %d %d\n", "Activation date", nvs_act_data.start_date, nvs_act_data.end_date);
		// ksceKernelPrintf("%31s 0x%llX\n", "ToolSecureTick", tick.tick);
		ksceKernelPrintf("%31s %04d/%02d/%02d %02d:%02d:%02d (%d %d)\n",
			"current ToolSecureTick",
			date_time.year, date_time.month, date_time.day, date_time.hour, date_time.minute, date_time.second,
			unixtime[0], unixtime[1]
		);

		{
			SceUInt64 startDate0 = ((SceUInt64)nvs_act_data.end_date * 1000000ULL) + 0xDCBFFEFF2BC000LL; // unixtime to sce tick
			tick.tick = startDate0 - tick.tick;
			ksceRtcConvertTickToDateTime(&date_time, &tick);
			ksceKernelPrintf("Activation expires in %d-day +%02d:%02d:%02d\n", date_time.day - 1, date_time.hour, date_time.minute, date_time.second);
		}

		set_tool_secure_tick((SceUInt64)nvs_act_data.start_date);
		ksceKernelPrintf("Doen setting activation start_date to ToolSecureTick\n");
	}

	return 0;
}

void _start() __attribute__((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){
	program_main();
	return SCE_KERNEL_START_NO_RESIDENT;
}
