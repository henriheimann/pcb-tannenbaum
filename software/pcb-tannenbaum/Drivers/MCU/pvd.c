#include <stm32l0xx_ll_utils.h>
#include "pvd.h"

uint8_t pvd_get_voltage()
{
	uint8_t pvd_level = 0;

	while (pvd_level <= 6) {
		LL_PWR_DisablePVD();
		LL_PWR_SetPVDLevel(pvd_level << 5u);
		LL_PWR_EnablePVD();
		LL_mDelay(1);
		if (LL_PWR_IsActiveFlag_PVDO()) {
			return 19 + pvd_level * 2;
		}
		pvd_level++;
	}

	return 33;
}