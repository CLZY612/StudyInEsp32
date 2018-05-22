#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "esp_timer.h"

//��̬����2����ʱ���Ļص�����
void test_timer_periodic_cb(void *arg);
void test_timer_once_cb(void *arg);

//����2����ʱ�����
esp_timer_handle_t test_p_handle = 0;
esp_timer_handle_t test_o_handle = 0;

//����һ���������еĶ�ʱ���ṹ��
esp_timer_create_args_t test_once_arg = { .callback = &test_timer_once_cb, //���ûص�����
		.arg = NULL, //��Я������
		.name = "TestOnceTimer" //��ʱ������
		};
//����һ�������ظ����еĶ�ʱ���ṹ��
esp_timer_create_args_t test_periodic_arg = { .callback =
		&test_timer_periodic_cb, //���ûص�����
		.arg = NULL, //��Я������
		.name = "TestPeriodicTimer" //��ʱ������

		};

void test_timer_periodic_cb(void *arg) {

	int64_t tick = esp_timer_get_time();

	printf("�����ص�����: %s , ���붨ʱ������ʱ���� = %lld \r\n", __func__, tick);

	if (tick > 100000000) {
		//ֹͣ��ʱ������������ȡ�Ƿ�ֹͣ�ɹ�
		esp_err_t err = esp_timer_stop(test_p_handle);
		printf("Ҫֹͣ�Ķ�ʱ�����֣�%s , �Ƿ�ֹͣ�ɹ���%s", test_periodic_arg.name,
				err == ESP_OK ? "ok!\r\n" : "failed!\r\n");
		err = esp_timer_delete(test_p_handle);
		printf("Ҫɾ���Ķ�ʱ�����֣�%s , �Ƿ�ֹͣ�ɹ���%s", test_periodic_arg.name,
				err == ESP_OK ? "ok!\r\n" : "failed!\r\n");
	}

	//�͵�ƽ
	gpio_set_level(16, 0);
	//�ӳ�
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	//�ߵ�ƽ
	gpio_set_level(16, 1);
	//�ӳ�
	vTaskDelay(1000 / portTICK_PERIOD_MS);

}

void test_timer_once_cb(void *arg) {

	int64_t tick = esp_timer_get_time();

	printf("�����ص�����: %s , ���붨ʱ������ʱ���� = %lld \r\n", __func__, tick);

	esp_err_t err = esp_timer_delete(test_o_handle);

	printf("Ҫɾ���Ķ�ʱ�����֣�%s , �Ƿ�ֹͣ�ɹ���%s", test_periodic_arg.name,
			err == ESP_OK ? "ok!\r\n" : "failed!\r\n");

}

void app_main() {

	gpio_pad_select_gpio(16);
	gpio_set_direction(16, GPIO_MODE_OUTPUT);

	//��ʼ����һ���ظ����ڵĶ�ʱ������ִ��
	esp_err_t err = esp_timer_create(&test_periodic_arg, &test_p_handle);
	err = esp_timer_start_periodic(test_p_handle, 1000 * 1000);
	printf("�ظ��������еĶ�ʱ������״̬��: %s", err == ESP_OK ? "ok!\r\n" : "failed!\r\n");

	//��ʼ����һ���������ڵĶ�ʱ������ִ��
	err = esp_timer_create(&test_once_arg, &test_o_handle);
	err = esp_timer_start_once(test_o_handle, 10 * 1000 * 1000);
	printf("�������еĶ�ʱ������״̬��: %s", err == ESP_OK ? "ok!\r\n" : "failed!\r\n");

}

