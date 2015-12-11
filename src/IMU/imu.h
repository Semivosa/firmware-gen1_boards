/*  Copyright (C) 2015 Joerg Hoener

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/**
 * Select one define for data transfer mode
 * and used filter
 */
//#define IMU_MODE_POLL 1
#define IMU_MODE_DMA 1


#define IMU_FILTER_COMPL 
//#define IMU_FILTER_COMPL2
//#define IMU_FILTER_KALMAN

/**
 * Define the used IMU from all that are implemented 
 */
#define IMU_IS_MPU6050
//#define IMU_IS_...

/**
 * Initialize IMU to be used for data transfer
 *
 * @param [in] func*(float*, float*) Callback for new data arrival
 *                            in DMA mode,
 *           @param [out] float* Angle in rad of forward backward lean.
 *           @param [out] * Sideway fall over is indicated as follows:
 *                          0 = Between +-45 degree
 *                          1 = Leaned more than 45 degree right
 *                          2 = Leaned more than 45 degree left 
 *
 * @return BOOL Indicate whether initialisation 
 */
#ifdef IMU_MODE_POLL 
BOOL IMU_init();

void IMU_getData(float* fwBkAngle, uint8_t* sideLean);

#elif IMU_MODE_DMA
BOOL IMU_init(void (*dataAvailable)(float*, uint8_t*));

void IMU_startDMAtransfer();

#endif

/**
 * Filter interface, implemented as per above filter define
 */
float callFilter(float oldAngle, float newAngle, float newRate,int looptime);