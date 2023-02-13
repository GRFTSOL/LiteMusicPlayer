import { reactive } from 'vue'
import { registerHandler, sendPlayerCommand, TYPE_PLAYER_STATES, TYPE_PLAYER_NOTIFICATION, TYPE_PLAYER_REMOTE_CTRL } from 'src/boot/web_socket_client';
import { deepCopy, assert } from 'src/boot/utils';


let isPlaying = false;
let startPosition = 0, startPositionTime = new Date().getTime(), idUpdatePlayerPosition;

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
registerHandler(TYPE_PLAYER_REMOTE_CTRL, handlePlayerRemoteCtrlResult);

function handlePlayerStateMsg(json) {
    let status = json.status;
    if (status != null) {
        if (status === 'playing') {
            if (!isPlaying) {
                isPlaying = true;
                idUpdatePlayerPosition = setInterval(updatePlayerPosition, 1000);
                startPositionTime = new Date().getTime();
            }
        } else {
            if (isPlaying) {
                isPlaying = false;
                clearInterval(idUpdatePlayerPosition);

                let now = new Date().getTime();;
                startPosition += now - startPositionTime;
                startPositionTime = now;
            }
        }
    }

    if (json.position != null) {
        startPosition = json.position;
        startPositionTime = new Date().getTime();
    }

    deepCopy(playerData, json);

    console.log(JSON.stringify(json, null, 2));
    console.log(json.cur_media?.title);
    console.log(playerData.cur_media.title);
}

function handlePlayerRemoteCtrlResult(json) {
    // No need to handle.
    assert(1);
}

function updatePlayerPosition() {
    playerData.position = startPosition + new Date().getTime() - startPositionTime;
}