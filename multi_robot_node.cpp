#include <ros/ros.h>
#include <std_msgs/String.h>
#include "multi_robot_package/Position.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "generate_prompt.h"


/* A constant array that holds the first half of each prompt */
const std::string front_prompt[] = {R"(## Building Layout\n\n### Initial Position\n- **Current Position:** (Facing North at the entrance)\n\n### Desired Position\n- **Destination:**)",
                                    R"(## Building Layout\n\n### Initial Position\n- **Current Position:** (Facing East at the East intersection)\n\n### Desired Position\n- **Destination:**)",
                                    R"(## Building Layout\n\n### Initial Position\n- **Current Position:** (Facing West at the West T intersection)\n\n### Desired Position\n- **Destination:**)"};
/* index 0: the entrance prompt
   index 1: the right intersection prompt
   index 2: the left intersection prompt */

/* A constant array that holds the second half of each prompt */
const std::string back_prompt[] = {R"(\n\n### Right Hallway (Facing North at the entrance)\n- **Room Sequence:**\n1. 2.106 (Left)\n2. 2.102 (Right)\n3. 2.108 (Left)\n4. 2.110 (Left)\n5. 2.112 (Left)\n6. 2.114 (Left)\n- **Second Hallway (Turning left at the end of the Right Hallway)**\n**Room Sequence:**\n1. 2.202 (Right)\n2. 2.204 (Left)\n3. 2.206 (Left)\n4. 2.208 (Left)\n\n### Left Hallway (Facing North at the entrance)\n- **Room Sequence:**\n1. 2.128 (Right)\n2. 2.116 (Left)\n3. 2.124 (Right)\n4. 2.122 (Right)\n5. 2.120 (Right)\n\n\n- **Continuation AFTER (going straight) the T intersection:**\n**Room Sequence:**\n1. 2.302 (Left)\n2. 2.308 (Right)\n3. 2.306 (Right)\n\n- **Right Turn at the T intersection:**\n**Room Sequence:**\n1. 2.328 (Right)\n2. 2.326 (Right)\n3. 2.304 (Left)\n4. 2.324 (Right)\n5. 2.322 (Right)\n6. 2.304 (Left, duplicate room entrance)\n7. 2.320 (Right)\n\n\nOUTPUT DIRECTIONS FROM INITIAL POSITION TO DESIRED POSITION IN BULLETS, each bullet has to be a sentence. ONLY OUTPUT THESE BULLETED DIRECTIONS, NOTHING ELSE.\n)",
                                   R"(\n\n### Left Hallway (Facing East at the East intersection)\n- **Room Sequence:**\n1. 2.204 (Left)\n2. 2.202 (Right)\n3. 2.206 (Left)\n4. 2.208 (Left)\n\n### Hallway behind you\n- **Room Sequence:**\n1. 2.114 (Right)\n2. 2.112 (Right)\n3. 2.110 (Right)\n4. 2.108 (Right)\n5. 2.102 (Left)\n6. 2.106 (Right)\n**Main atrium splits this hallway briefly**\n7. 2.128 (Right)\n8. 2.116 (Left)\n9. 2.124 (Right)\n10. 2.122 (Right)\n11. 2.120 (Right)\n12. T intersection of Hallway behind you with a small continuation hallway and a hallway to the right\n\n\n- **Continuation AFTER (going straight) the T intersection:**\n**Room Sequence:**\n1. 2.302 (Left)\n2. 2.308 (Right)\n3. 2.306 (Right)\n\n- **Right Turn at the T intersection:**\n**Room Sequence:**\n1. 2.328 (Right)\n2. 2.326 (Right)\n3. 2.304 (Left)\n4. 2.324 (Right)\n5. 2.322 (Right)\n6. 2.304 (Left, duplicate room entrance)\n7. 2.320 (Right)\n\nOUTPUT DIRECTIONS FROM INITIAL POSITION TO DESIRED POSITION IN BULLETS, each bullet has to be a sentence. ONLY OUTPUT THESE BULLETED DIRECTIONS, NOTHING ELSE.\n)",
                                   R"(\n\n### continuation of this same hallway, going straight at this t intersection\n**Room Sequence:**\n1. 2.302 (Left)\n2. 2.308 (Right)\n3. 2.306 (Right)\n\n### hallway turning right at this t intersection\n**Room Sequence:**\n1. 2.328 (Right)\n2. 2.326 (Right)\n3. 2.304 (Left)\n4. 2.324 (Right)\n5. 2.322 (Right)\n6. 2.304 (Left, duplicate room entrance)\n7. 2.320 (Right)\n\n### Hallway behind you\n- **Room Sequence:**\n1. 2.120 (Left)\n2. 2.122 (Left)\n3. 2.124 (Left)\n4. 2.116 (Right)\n5. 2.128 (Left)\n**Main atrium splits this hallway briefly**\n6. 2.106 (Left)\n7. 2.102 (Right)\n8. 2.108 (Left)\n9. 2.110 (Left)\n10. 2.112 (Left)\n11. 2.114 (Left)\n\n### Left Hallway (at the end of the hallway behind you)\n- **Room Sequence:**\n1. 2.204 (Left)\n2. 2.202 (Right)\n3. 2.206 (Left)\n4. 2.208 (Left)\n\nOUTPUT DIRECTIONS FROM INITIAL POSITION TO DESIRED POSITION IN BULLETS, each bullet has to be a sentence. ONLY OUTPUT THESE BULLETED DIRECTIONS, NOTHING ELSE.\n)"};
/* The indexes are the same as above */

/* stores the "location" of the robot, this is used to index into the prompt arrays */
int curr_location = 0;


// This function replaces all occurrences of a substring with another substring
void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

// Function to find and extract content between two delimiters
std::string extractContent(const std::string &source, const std::string &start_delim,
                           const std::string &end_delim)
{
    size_t start = source.find(start_delim); // Find the first delimiter
    if (start == std::string::npos)
        return ""; // Didn't find it, return empty string

    start += start_delim.length();              // Move past the delimiter
    size_t end = source.find(end_delim, start); // Find the next delimiter
    if (end == std::string::npos)
        return ""; // Didn't find it, return empty string

    return source.substr(start, end - start); // Extract the content
}

// Callback function writes data to the variable pointed to by the userp parameter
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    userp->append((char *)contents, size * nmemb);
    return size * nmemb;
}

int main(int argc, char **argv)
{
    std::string url = "https://api.openai.com/v1/chat/completions";
    std::string apiKey = "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"; // replace with your actual OpenAI API key

    CURL *curl = curl_easy_init();
    CURLcode res;
    std::string readBuffer;

    ros::init(argc, argv, "multi_robot_node");
    ros::NodeHandle nh;

    BuildingMap ahg = init_map();

    // Add your ROS publishers and subscribers here if needed
    ros::Publisher directions_pub = nh.advertise<std_msgs::String>("directions", 10);
    if (curl)
    {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set up headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, auth.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string roomNumber = "";

        while (ros::ok())
        {
            // Ask for room number or exit condition
            std::cout << "Enter the destination room number (or 'E' to exit): " << roomNumber << std::endl;
            std::getline(std::cin, roomNumber);

            if (roomNumber == "E")
            {
                break; // Exit the loop and node
            } else {
                curr_location = Position::returnNewPosition();
                std::cout << "New position: " << curr_location << std::endl;
                continue;
            }

            /* have to get a way to determine the orientation and translate it
               to the Direction enum. */
            vector<string> prompt = generate_prompt(roomNumber, NORTH, ahg);

            // Prepare the postData with the user's room number
        //     std::string postData = R"({
        //     "model": "gpt-4",
        //     "messages": [
        //         {"role": "system", "content": "You are a helpful assistant."},
        //         {"role": "user", "content": ")" + front_prompt[curr_location] + roomNumber + back_prompt[curr_location] + R"("}
        //     ],
        //     "max_tokens": 200
        // })";
            std::string postData = R"({
            "model": "gpt-4",
            "messages": [
                {"role": "system", "content": "You are a helpful assistant."},
                {"role": "user", "content": ")" + prompt[0] + roomNumber + prompt[1] + R"("}
            ],
            "max_tokens": 200
        })";

            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

            // Set up the write function to handle the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

            // Perform the request, res will get the return code
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK)
            {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
            else
            {
                // Here, we're extracting the content field from the response.
                std::string contentField = "\"content\": \""; // This is how the content field should start
                std::string endDelim = "\"";                  // The content field ends with a double quote

                // Extract the actual message content
                std::string messageContent = extractContent(readBuffer, contentField, endDelim);

                // If we found something, print it
                if (!messageContent.empty())
                {
                    std::string contentField = "\"content\": \""; // This is how the content field should start
                    std::string endDelim = "\"";                  // The content field ends with a double quote

                    // Extract the actual message content
                    std::string messageContent = extractContent(readBuffer, contentField, endDelim);

                    // Replace escaped newlines with actual newlines
                    replaceAll(messageContent, "\\n", "\n");
                    if (!messageContent.empty())
                    {
                        // Publish the directions as a ROS message
                        std_msgs::String directions_msg;
                        directions_msg.data = "Directions: \n" + messageContent; // messageContent contains the extracted directions
                        directions_pub.publish(directions_msg);

                        // Log the directions to the console
                        ROS_INFO("%s", directions_msg.data.c_str());
                    }
                    else
                    {
                        std::cerr << "Could not find content in the response." << std::endl;
                    }
                }
                else
                {
                    std::cerr << "Could not find content in the response." << std::endl;
                }
            }

            // Optional: clear the readBuffer before the next iteration
            readBuffer.clear();
        }
        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else
    {
        std::cerr << "Error initializing curl." << std::endl;
        return 1;
    }
    return 0;
}
