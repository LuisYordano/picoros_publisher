#include <cstdio>
#include <cstdint>
#include "picoros.h"
#include "picoserdes.h"

constexpr const char *MODE = "client";
constexpr const char *LOCATOR = "serial/UART_0#baudrate=460800";

constexpr const char *NODE_NAME = "talker";
constexpr const char *TOPIC_NAME = "picoros/chatter";
constexpr const char *MESSAGE = "Hello from Pico-ROS";

class PicoRosTalker
{
public:
    PicoRosTalker(const char *mode, const char *locator, const char *node_name, const char *topic_name) : message_count(0)
    {
        ifx = {
            .mode = const_cast<char *>(mode),
            .locator = const_cast<char *>(locator),
        };

        node = {
            .name = node_name,
        };

        pub_log = {
            .topic = {
                .name = topic_name,
                .type = ROSTYPE_NAME(ros_String),
                .rihs_hash = ROSTYPE_HASH(ros_String),
            },
        };
    }

    void init()
    {
        std::printf("Starting pico-ros interface %s %s\n", ifx.mode, ifx.locator);
        while (picoros_interface_init(&ifx) == PICOROS_NOT_READY)
        {
            std::printf("Waiting RMW init...\n");
            z_sleep_s(1);
        }

        std::printf("Starting Pico-ROS node %s domain:%lu\n", node.name, node.domain_id);
        picoros_node_init(&node);

        std::printf("Declaring publisher on %s\n", pub_log.topic.name);
        picoros_publisher_declare(&node, &pub_log);
    }

    void publish(const char *msg)
    {
        std::printf("Publishing: %s\n", msg);
        size_t len = ps_serialize(pub_buf, (char **)&msg, sizeof(pub_buf));
        picoros_publish(&pub_log, pub_buf, len);
    }

    void run()
    {
        char message_buffer[100];
        while (true)
        {
            snprintf(message_buffer, sizeof(message_buffer), "%s: %d", MESSAGE, message_count++);
            publish(message_buffer);
            z_sleep_s(1);
        }
    }

private:
    picoros_interface_t ifx;
    picoros_node_t node;
    picoros_publisher_t pub_log;
    uint8_t pub_buf[1024];
    int message_count;
};

PicoRosTalker talker(MODE, LOCATOR, NODE_NAME, TOPIC_NAME);

extern "C" void app_main(void)
{
    talker.init();
    talker.run();
}
