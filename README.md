Mars Photoshooting
================

Service can store/load data in a PostgresDB. The main feature is uploading (netbpm) images and applying filters to it.

Two bugs lead to the control flow jumping to the image data allowing RCE, but only after the data was manipulated with the filters.

*Bug 1*: Incorrect argument parsing allows to specify the use of 4 filters while only providing 3. Exploiting this bugs requires 'watermark-resistent' shellcode.
*Bug 2*: Problems with parsing the config value allows to use the config values for 2021 which has a larger blur size. This allows to another RCE due to a bug in the blur code, but requires 'blur-resistent' shellcode.

The main challange of this service is to generate images that form valid shellcode *after* applying the watermark resp. blur filter.
*Watermark*: Several (predefined) pixels get erased. Solution: Use the pixels not modified and insert short jumps between those gaps. Only restriction to the shellcode is to use instructions not larger than 9 Bytes.
*Blur*: Typical 3x3 blur filter. Each pixel in the resulting image depends on 9 pixels in the original image.
Furthermore, two adjacent pixels have 6 pixels in the original image in common.
This imposes limitations on the shellcode. E.g. the absolut difference between two adjacent pixels in the resulting shellcode cannot be too large.
Several tricks can help:
- use short instructions and do some forward jumps between those instructions to 'break dependency chains'
- use operations serving as 'nop's for filling if required
- only use every third row in the image to avoid vertical dependencies

Sellcode Content:
Perform an SQL query by calling queryString(char *) with the query string of your choice and write the result back to the image buffer. It will be sent back to the client.

