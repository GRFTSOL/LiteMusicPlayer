import { reactive } from 'vue'
import { registerHandler, sendPlayerCommand, TYPE_PLAYER_STATES, TYPE_PLAYER_NOTIFICATION } from 'src/boot/web_socket_client';
import { deepCopy } from 'src/boot/utils';


// 存储了 player 的所有信息：参考 PlayerEventSender.cpp 传递的数据结构
export let playerData = reactive({
    status: 'stopped',
    position: 0,
    cur_playlist: [
        {
            id: 0,
            artist: '',
            album: '',
            title: '',
            file: '',
            duration: 0,
        }
    ],
    cur_media: {
        id: 0,
        artist: '',
        album: '',
        title: '',
        file: '',
        duration: 0,
    },
    settings: {
        shuffle: false,
        loop: 'all',
        mute: false,
        volume: 50,
    },

    // Update values
    setVolume(value) {
        sendPlayerCommand('settings.volume', value);
    },
    seek(value) {
        sendPlayerCommand('position', value);
    },
});
window._playerData = playerData;

registerHandler(TYPE_PLAYER_STATES, handlePlayerStateMsg);
registerHandler(TYPE_PLAYER_NOTIFICATION, handlePlayerStateMsg);

function handlePlayerStateMsg(json) {
    deepCopy(playerData, json);
}