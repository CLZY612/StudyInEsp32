/* GPIO Example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_log.h"

typedef enum {
	KEY_SHORT_PRESS = 1, KEY_LONG_PRESS,
} alink_key_t;

//�궨��һ��GPIO
#define KEY_GPIO    14

static xQueueHandle gpio_evt_queue = NULL;

void IRAM_ATTR gpio_isr_handler(void *arg) {
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void KeyInit(uint32_t key_gpio_pin) {

	//����GPIO���½��غ������ش����ж�
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.pin_bit_mask = 1 << key_gpio_pin;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	gpio_set_intr_type(key_gpio_pin, GPIO_INTR_ANYEDGE);
	gpio_evt_queue = xQueueCreate(2, sizeof(uint32_t));

	gpio_install_isr_service(0);
	gpio_isr_handler_add(key_gpio_pin, gpio_isr_handler, (void *) key_gpio_pin);
}

esp_err_t alink_key_scan(TickType_t ticks_to_wait) {

	uint32_t io_num;
	BaseType_t press_key = pdFALSE;
	BaseType_t lift_key = pdFALSE;
	int backup_time = 0;

	while (1) {

		//���մ���Ϣ���з�������Ϣ
		xQueueReceive(gpio_evt_queue, &io_num, ticks_to_wait);

		//��¼���û����°�����ʱ���
		if (gpio_get_level(io_num) == 0) {
			press_key = pdTRUE;
			backup_time = system_get_time();
			//�����ǰGPIO�ڵĵ�ƽ�Ѿ���¼Ϊ���£���ʼ��ȥ�ϴΰ��°�����ʱ���
		} else if (press_key) {
			//��¼̧��ʱ���
			lift_key = pdTRUE;
			backup_time = system_get_time() - backup_time;
		}

		//�������±�־λ�Ͱ��������־λ��Ϊ1ʱ�򣬲�ִ�лص�
		if (press_key & lift_key) {
			press_key = pdFALSE;
			lift_key = pdFALSE;

			//�������1s��ص�����������Ͷ̰��ص�
			if (backup_time > 1000000) {
				return KEY_LONG_PRESS;
			} else {
				return KEY_SHORT_PRESS;
			}
		}
	}
}

void key_trigger(void *arg) {
	esp_err_t ret = 0;
	KeyInit(KEY_GPIO);

	while (1) {
		ret = alink_key_scan(portMAX_DELAY);
		if (ret == -1)
			vTaskDelete(NULL);

		switch (ret) {
		case KEY_SHORT_PRESS:
			printf("�̰������ص� ... \r\n");
			break;

		case KEY_LONG_PRESS:
			printf("���������ص� ... \r\n");
			break;

		default:
			break;
		}
	}

	vTaskDelete(NULL);
}

void app_main() {
	xTaskCreate(key_trigger, "key_trigger", 1024 * 2, NULL, 10,
	NULL);
}

