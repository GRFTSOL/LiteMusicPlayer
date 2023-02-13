import { reactive } from 'vue'
import { registerHandler, sendPlayerCommand, TYPE_MEDIA_LIB_NOTIFICATION, TYPE_MEDIA_LIB_ALL } from 'src/boot/web_socket_client';


const MNC_UPDATE = 1,
    MNC_REMOVE = 2,
    MNC_INSERT = 3;

// 存储了 MediaLibrary 的所有信息
let mediaLibrary = reactive({
    list: [
        {
            id: 0,
            artist: '',
            album: '',
            title: '',
            file: '',
            duration: 0,
            genre: '',
            year: '',
        }
    ],
});
window._mediaLibrary = mediaLibrary;

registerHandler(TYPE_MEDIA_LIB_NOTIFICATION, handleMediaLibNotification);
registerHandler(TYPE_MEDIA_LIB_ALL, handleMediaLibAll);

function handleMediaLibNotification(json) {
    switch (json.cmd) {
        case MNC_UPDATE:
            break;
    }
}

function handleMediaLibAll(json) {
    for (let k in json) {
        mediaLibrary[k] = json[k];
    }
}

export function getMediaLibrary() {
    return mediaLibrary;
}