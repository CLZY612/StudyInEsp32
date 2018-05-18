#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

//�궨��һ��GPIO������GPIO�ڵ��������
#define BLINK_GPIO 16

//����GPIO�ߵ͵�ƽ���
void fun_set_gpio_level() {

	//��һ�ַ�ʽ����
	//gpio_pad_select_gpio(BLINK_GPIO);
	//gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	//�ڶ��ַ�ʽ����
	gpio_config_t io_conf;
	//�����ж�
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//ѡ��Ϊ���ģʽ
	io_conf.mode = GPIO_MODE_OUTPUT;
	//����GPIO_OUT�Ĵ���
	io_conf.pin_bit_mask = GPIO_SEL_16;
	//��ֹ����
	io_conf.pull_down_en = 0;
	//��ֹ����
	io_conf.pull_up_en = 0;
	//�������ʹ��
	gpio_config(&io_conf);

	//����͵�ƽ
	gpio_set_level(BLINK_GPIO, 0);

}

//���û�ȡָ����GPIO�������ƽ
void fun_get_gpio_level() {

	//��һ�ַ�ʽ����
	//gpio_pad_select_gpio(BLINK_GPIO);
	//gpio_set_direction(BLINK_GPIO, GPIO_MODE_INPUT);

	//�ڶ��ַ�ʽ����
	gpio_config_t io_conf;
	//�����ж�
	io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
	//ѡ��Ϊ���ģʽ
	io_conf.mode = GPIO_MODE_INPUT;
	//����GPIO_OUT�Ĵ���
	io_conf.pin_bit_mask = GPIO_SEL_16;
	//��ֹ����
	io_conf.pull_down_en = 0;
	//��ֹ����
	io_conf.pull_up_en = 0;
	//�������ʹ��
	gpio_config(&io_conf);

	/* ����500ms. */
	const portTickType xDelay = 500 / portTICK_RATE_MS;

	while (1) {
		printf(" Current Gpio16 Level is : %d \r\n\r\n",
				gpio_get_level(BLINK_GPIO));
		vTaskDelay(xDelay);
	}

}

/***************************************************************************************/
/*************            �������жϺ�����ʹ��                                        *****************************/
/***************************************************************************************/

#define GPIO_INPUT_IO_0     4

static xQueueHandle gpio_evt_queue = NULL; //����һ�����з��ر���

void IRAM_ATTR gpio_isr_handler(void* arg) {
	//���ж���Ϣ���뵽���еĺ��棬��gpio��io�������ݵ�������
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

//�͵�ƽ�����Ļص�����
void gpio_low_interrupt_callBack(void* arg) {
	printf(" \r\n into gpio_low_interrupt_callBack ...\r\n  ");
	uint32_t io_num;
	while (1) {
		//���϶�ȡgpio���У���ȡ���ɾ������
		if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			printf("GPIO[%d] �жϴ���, ��ǰ�ĵ�ѹ: %d\n", io_num,
					gpio_get_level(io_num));
		}
	}
}

void fun_set_gpio_low_interrupt() {

	//GPIO�ڽṹ�嶨��
	gpio_config_t io_conf;
	//�½��ش���
	io_conf.intr_type = GPIO_INTR_NEGEDGE;
	//ѡ��Ϊ���ģʽ
	io_conf.mode = GPIO_MODE_INPUT;
	//����GPIO_OUT�Ĵ���
	io_conf.pin_bit_mask = GPIO_SEL_4;
	//��������
	io_conf.pull_down_en = 0;
	//��������
	io_conf.pull_up_en = 1;
	//�������ʹ��
	gpio_config(&io_conf);

	//ע���жϷ���
	gpio_install_isr_service(1);
	//����GPIO���жϻص�����
	gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler,
			(void*) GPIO_INPUT_IO_0);

	//����һ����Ϣ���У����л�ȡ���о��
	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	//�½����е�
	xTaskCreate(gpio_low_interrupt_callBack //������
			, "gpio_task_example" //��������
			, 2048  //�����ջ��С
			, NULL  //���ݸ��������Ĳ���
			, 10   //�������ȼ�
			, NULL); //�΄վ��

}

void app_main() {

	//����gpio�ĵ�ƽ���
	//fun_set_gpio_level();

	//��ȡgpio�ĵ�ƽ����
	//fun_get_gpio_level();

	//gpio�ж�
	fun_set_gpio_low_interrupt();
}

