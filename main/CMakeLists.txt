idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES
        "AWS_IOT_Cert/AmazonRootCA1.pem"
        "AWS_IOT_Cert/Device_Certificate.crt"
        "AWS_IOT_Cert/Private_Key.key"
    ) 