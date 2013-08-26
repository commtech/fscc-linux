/*! \file */

/*
    Copyright (C) 2013 Commtech, Inc.

    This file is part of fscc-linux.

    fscc-linux is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fscc-linux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <fcntl.h> /* open, O_RDWR */
#include <unistd.h> /* write, close */

#include "fscc.h"
#include "errno.h"
#include "calculate-clock-bits.h"

#define MAX_NAME_LENGTH 25

/******************************************************************************/
/*!

  \brief Opens a handle to an FSCC port.

  \param[in] port_num
    the FSCC port number
  \param[in] overlapped
    whether you would like to use the port in overlapped mode
  \param[out] h
    user variable that the port's HANDLE will be assigned to

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \note
    Opening a handle using this API will only give you access to the
    synchronous functionality of the card. You will need to use the COM ports
    if you would like to use the asynchronous functionality.

*/
/******************************************************************************/
int fscc_connect(unsigned port_num, int *fd)
{
    char name[MAX_NAME_LENGTH];

    sprintf(name, "/dev/fscc%u", port_num);

    *fd = open(name, O_RDWR);

    return (*fd != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Sets the transmit modifiers for the port.

  \param[in] h
    HANDLE to the port
  \param[in] modifiers
    bit mask of the transmit modifier values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \note
    XF - Normal transmit - disable modifiers
    XREP - Transmit repeat
    TXT - Transmit on timer
    TXEXT - Transmit on external signal

  \snippet set-tx-modifiers.c Set TXT | XREP
  \snippet set-tx-modifiers.c Set XF

*/
/******************************************************************************/
int fscc_set_tx_modifiers(int fd, unsigned modifiers)
{
    int result;

    result = ioctl(fd, FSCC_SET_TX_MODIFIERS, modifiers);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets the transmit modifiers for the port.

  \param[in] h
    HANDLE to the port
  \param[out] modifiers
    bit mask of the transmit modifier values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \note
    XF - Normal transmit - disable modifiers
    XREP - Transmit repeat
    TXT - Transmit on timer
    TXEXT - Transmit on external signal

  \snippet get-tx-modifiers.c Setup variables
  \snippet get-tx-modifiers.c Get modifiers

*/
/******************************************************************************/
int fscc_get_tx_modifiers(int fd, unsigned *modifiers)
{
    int result;

    result = ioctl(fd, FSCC_GET_TX_MODIFIERS, modifiers);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Sets the FSCC driver's memory caps.

  \param[in] h
    HANDLE to the port
  \param[in] memcap
    input and output memory cap values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-memory-cap.c Setup variables
  \snippet set-memory-cap.c Set memory cap

*/
/******************************************************************************/
int fscc_set_memory_cap(int fd, const struct fscc_memory_cap *memcap)
{
    int result;

    result = ioctl(fd, FSCC_SET_MEMORY_CAP, memcap);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets the FSCC driver's memory caps.

  \param[in] h
    HANDLE to the port
  \param[in] memcap
    input and output memory cap values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet get-memory-cap.c Setup variables
  \snippet get-memory-cap.c Get memory cap

*/
/******************************************************************************/
int fscc_get_memory_cap(int fd, struct fscc_memory_cap *memcap)
{
    int result;

    result = ioctl(fd, FSCC_GET_MEMORY_CAP, memcap);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Sets a port's register values.

  \param[in] h
    HANDLE to the port
  \param[in] regs
    the new register values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-registers.c Setup variables
  \snippet set-registers.c Set registers

*/
/******************************************************************************/
int fscc_set_registers(int fd, const struct fscc_registers *regs)
{
    int result;

    result = ioctl(fd, FSCC_SET_REGISTERS, regs);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets a port's register values.

  \param[in] h
    HANDLE to the port
  \param[out] regs
    the register values

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet get-registers.c Setup variables
  \snippet get-registers.c Get registers

*/
/******************************************************************************/
int fscc_get_registers(int fd, struct fscc_registers *regs)
{
    int result;

    result = ioctl(fd, FSCC_GET_REGISTERS, regs);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets a port's append status value.

  \param[in] h
    HANDLE to the port
  \param[out] status
    the append status value

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet get-append-status.c Setup variables
  \snippet get-append-status.c Get append status

*/
/******************************************************************************/
int fscc_get_append_status(int fd, unsigned *status)
{
    int result;

    result = ioctl(fd, FSCC_GET_APPEND_STATUS, status);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Enable appending the status to the received data.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-append-status.c Enable append status

*/
/******************************************************************************/
int fscc_enable_append_status(int fd)
{
    int result;

    result = ioctl(fd, FSCC_ENABLE_APPEND_STATUS);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Disable appending the status to the received data.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-append-status.c Disable append status

*/
/******************************************************************************/
int fscc_disable_append_status(int fd)
{
    int result;

    result = ioctl(fd, FSCC_DISABLE_APPEND_STATUS);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets a port's append timestamp value.

  \param[in] h
    HANDLE to the port
  \param[out] status
    the append timestamp value

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_get_append_timestamp(int fd, unsigned *timestamp)
{
    int result;

    result = ioctl(fd, FSCC_GET_APPEND_TIMESTAMP, timestamp);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Enable appending the timestamp to the received data.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_enable_append_timestamp(int fd)
{
    int result;

    result = ioctl(fd, FSCC_ENABLE_APPEND_TIMESTAMP);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Disable appending the timestamp to the received data.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_disable_append_timestamp(int fd)
{
    int result;

    result = ioctl(fd, FSCC_DISABLE_APPEND_TIMESTAMP);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Gets a port's ignore timeout value.

  \param[in] h
    HANDLE to the port
  \param[out] status
    the append status value

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet get-ignore-timeout.c Setup variables
  \snippet get-ignore-timeout.c Get ignore timeout

*/
/******************************************************************************/
int fscc_get_ignore_timeout(int fd, unsigned *status)
{
    int result;

    result = ioctl(fd, FSCC_GET_IGNORE_TIMEOUT, status);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Ignore card timeouts.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-ignore-timeout.c Enable ignore timeout

*/
/******************************************************************************/
int fscc_enable_ignore_timeout(int fd)
{
    int result;

    result = ioctl(fd, FSCC_ENABLE_IGNORE_TIMEOUT);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Disable ignore timeout.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-ignore-timeout.c Disable ignore timeout

*/
/******************************************************************************/
int fscc_disable_ignore_timeout(int fd)
{
    int result;

    result = ioctl(fd, FSCC_DISABLE_IGNORE_TIMEOUT);

    return (result != -1) ? 0 : errno;
}
/******************************************************************************/
/*!

  \brief Gets a port's receive multiple value.

  \param[in] h
    HANDLE to the port
  \param[out] status
    the rx multiple value

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_get_rx_multiple(int fd, unsigned *status)
{
    int result;

    result = ioctl(fd, FSCC_GET_RX_MULTIPLE, status);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Receive frames in groups.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_enable_rx_multiple(int fd)
{
    int result;

    result = ioctl(fd, FSCC_ENABLE_RX_MULTIPLE);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Disable receive multiple.

  \param[in] h
    HANDLE to the port

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_disable_rx_multiple(int fd)
{
    int result;

    result = ioctl(fd, FSCC_DISABLE_RX_MULTIPLE);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Clears the transmit and/or receive data out of the card.

  \param[in] h
    HANDLE to the port
  \param[in] tx
    whether to clear the transmit data out
  \param[in] rx
    whether to clear the receive data out

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \note
    Any pending transmit data will not be transmited upon a purge.

  \note
    If you receive a ERROR_TIMEOUT you are likely at a speed too slow (~35 Hz)
        for this driver. You will need to contact Commtech to get a custom driver.

  \snippet purge.c Purge TX
  \snippet purge.c Purge RX
  \snippet purge.c Purge both TX & RX

*/
/******************************************************************************/
int fscc_purge(int fd, unsigned tx, unsigned rx)
{
    int result;

    if (tx) {
        result = ioctl(fd, FSCC_PURGE_TX);

        if (result == -1)
            return errno;
    }

    if (rx) {
        result = ioctl(fd, FSCC_PURGE_RX);

        if (result == -1)
            return errno;
    }

    return 0;
}

/******************************************************************************/
/*!

  \brief Sets a port's clock frequency.

  \param[in] h
    HANDLE to the port
  \param[in] frequency
    the new clock frequency
  \param[in] ppm
    TODO

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

  \snippet set-clock-frequency.c Set clock frequency

  \todo
    What should I do about PPM?

*/
/******************************************************************************/
int fscc_set_clock_frequency(int fd, unsigned frequency, unsigned ppm)
{
    unsigned char clock_bits[20];
    int result;

    calculate_clock_bits(frequency, ppm, clock_bits);

    result = ioctl(fd, FSCC_SET_CLOCK_BITS, &clock_bits);

    return (result != -1) ? 0 : errno;
}

/******************************************************************************/
/*!

  \brief Transmits data out of a port.

  \param[in] h
    HANDLE to the port
  \param[in] buf
    the buffer containing the data to transmit
  \param[in] size
    the number of bytes to transmit from 'buf'
  \param[out] bytes_written
    the input variable to store how many bytes were actually written
  \param[in,out] o
    OVERLAPPED structure for asynchronous operation

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_write(int fd, char *buf, unsigned size, unsigned *bytes_written)
{
    int ret;

    ret = write(fd, buf, size);

    if (ret == -1)
        return errno;

    *bytes_written = ret;

    return 0;
}

/******************************************************************************/
/*!

  \brief Reads data out of a port.

  \param[in] h
    HANDLE to the port
  \param[in] buf
    the input buffer used to store the receive data
  \param[in] size
    the maximum number of bytes to read in (typically sizeof(buf))
  \param[out] bytes_read
    the user variable to store how many bytes were actually read
  \param[in,out] o
    OVERLAPPED structure for asynchronous operation

  \return 0
    if the operation completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_read(int fd, char *buf, unsigned size, unsigned *bytes_read)
{
    int ret;

    ret = read(fd, buf, size);

    if (ret == -1)
        return errno;

    *bytes_read = ret;

    return 0;
}

/******************************************************************************/
/*!

  \brief Closes the handle to an FSCC port.

  \param[in] h
    HANDLE to the port

  \return 0
    if closing the port completed successfully
  \return >= 1
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int fscc_disconnect(int fd)
{
    int result;

    result = close(fd);

    return (result != -1) ? 0 : errno;
}
