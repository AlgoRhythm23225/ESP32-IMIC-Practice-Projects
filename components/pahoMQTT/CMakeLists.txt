idf_component_register(
    SRCS 
        "MQTTAsync.c" "MQTTPersistence.c" "Clients.c" "MQTTTime.c" "MQTTProperties.c" "StackTrace.c" "MQTTClient.c"
        "MQTTReasonCodes.c" "MQTTPacket.c" "LinkedList.c" "Log.c" "MQTTPacketOut.c" "MQTTAsyncUtils.c" "MQTTPersistenceDefault.c"
        "utf-8.c" "Thread.c" "Heap.c" "Messages.c" "MQTTProtocolClient.c" "MQTTProtocolOut.c" "WebSocket.c" "OsWrapper.c"
        "SocketBuffer.c" "Tree.c" "Proxy.c"
    INCLUDE_DIRS 
        "Include"
    REQUIRES) 

