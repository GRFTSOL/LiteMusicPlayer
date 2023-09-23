import { reactive } from 'vue'
import { registerHandler, TYPE_MEDIA_LIB_NOTIFICATION, TYPE_MEDIA_LIB_ALL } from 'src/boot/web_socket_client';
import { Media } from 'src/components/models';


const MNC_UPDATE = 1;
// const MNC_REMOVE = 2;
// const MNC_INSERT = 3;

const mediaLibChangedCallbacks: any = [];

// 存储了 MediaLibrary 的所有信息
const mediaLibrary = reactive({
    list: [] as Array<Media>,
    path_sep: '/',
});

(window as any)._mediaLibrary = mediaLibrary;

registerHandler(TYPE_MEDIA_LIB_NOTIFICATION, handleMediaLibNotification);
registerHandler(TYPE_MEDIA_LIB_ALL, handleMediaLibAll);

function handleMediaLibNotification(json: any) {
    switch (json.cmd) {
        case MNC_UPDATE:
            break;
    }
}

function handleMediaLibAll(json: any) {
    for (const k in json) {
        (mediaLibrary as any)[k] = json[k];
    }

    for (const callback of mediaLibChangedCallbacks) {
        callback();
    }
}

export function getMediaLibrary() {
    return mediaLibrary;
}

export function registerMediaLibChangedHandler(callback: any) {
    mediaLibChangedCallbacks.push(callback);
}

export function saveMediaBasicInfo(media: Media) {
    return true;
}