
unsigned char i2c_read(char *dev_name, unsigned char dev_addr, unsigned char reg_addr, unsigned char *data, unsigned short length)
{
    /* I2Cデバイスをオープンする. */
    int fd = open(dev_name, O_RDWR);
    if (fd == -1)
    {
        fprintf(stderr, "i2c_read: failed to open: %s\n", strerror(errno));
        return -1;
    }

    /* I2C-Readメッセージを作成する. */
    struct i2c_msg messages[] = {
        {dev_addr, 0, 1, &reg_addr},        /* レジスタアドレスをセット. */
        {dev_addr, I2C_M_RD, length, data}, /* dataにlengthバイト読み込む. */
    };
    struct i2c_rdwr_ioctl_data ioctl_data = {messages, 2};

    /* I2C-Readを行う. */
    if (ioctl(fd, I2C_RDWR, &ioctl_data) != 2)
    {
        fprintf(stderr, "i2c_read: failed to ioctl: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

unsigned char i2c_write(char *dev_name, unsigned char dev_addr, unsigned char reg_addr, const unsigned char *data, unsigned short length)
{
    /* I2Cデバイスをオープンする. */
    int fd = open(dev_name, O_RDWR);
    if (fd == -1)
    {
        fprintf(stderr, "i2c_write: failed to open: %s\n", strerror(errno));
        return -1;
    }

    /* I2C-Write用のバッファを準備する. */
    unsigned char *buffer = (unsigned char *)malloc(length + 1);
    if (buffer == NULL)
    {
        fprintf(stderr, "i2c_write: failed to memory allocate\n");
        close(fd);
        return -1;
    }
    buffer[0] = reg_addr;             /* 1バイト目にレジスタアドレスをセット. */
    memcpy(&buffer[1], data, length); /* 2バイト目以降にデータをセット. */

    /* I2C-Writeメッセージを作成する. */
    struct i2c_msg message = {dev_addr, 0, length + 1, buffer};
    struct i2c_rdwr_ioctl_data ioctl_data = {&message, 1};

    /* I2C-Writeを行う. */
    if (ioctl(fd, I2C_RDWR, &ioctl_data) != 1)
    {
        fprintf(stderr, "i2c_write: failed to ioctl: %s\n", strerror(errno));
        free(buffer);
        close(fd);
        return -1;
    }

    free(buffer);
    close(fd);
    return 0;
}