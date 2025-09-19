/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import { BusinessError } from '@ohos.base';
import Audio from '@ohos.multimedia.audio';
import Media from '@ohos.multimedia.media';
import type Want from '@ohos.app.ability.Want';
import Fs from '@ohos.file.fs';

const LOGTAG = 'PowerChargeSound'

export class ChargeSoundAbility extends ServiceExtensionAbility {
    private chargeSoundPlayer: ChargeSoundPlayer;
    
    onCreate(want: Want): void {
        console.info(LOGTAG, 'ChargeSoundAbility onCreate');
        try {
            let audioPath : string = want.parameters['audioPath'] as string;
            this.chargeSoundPlayer = new ChargeSoundPlayer(audioPath);
        } catch (e) {
            console.error(LOGTAG, `ChargeSoundAbility onCreate error: ${e?.message}`);
        }
    }
    onDestroy(): void {
        console.info(LOGTAG, 'ChargeSoundAbility onDestroy');
        if (this.chargeSoundPlayer == null) {
            console.info(LOGTAG, 'chargeSoundPlayer is null');
            return;
        }
        this.chargeSoundPlayer.stopChargeSound().then(() => {
            console.info(LOGTAG, 'ChargeSoundAbility onDestroy completed');
        }).catch((error) => {
            console.error(LOGTAG, `Error during onDestroy: ${error?.message}`);
        });
    }
    onRequest(want: Want, startId: number): void {
        console.info(LOGTAG, 'ChargeSoundAbility onRequest');
        if (this.chargeSoundPlayer == null) {
            console.error(LOGTAG, 'chargeSoundPlayer is null');
            return;
        }
        this.chargeSoundPlayer.startChargeSound();
    }
}


type OnResetCb = () => void;

/**
 * ChargeSound Player
 * @class ChargeSoundPlayer
 */
class ChargeSoundPlayer {
    private audioFile: Fs.File;

    private isAudioPlaying: boolean = false;

    private OnReset?: OnResetCb;

    constructor(audioPath: string) {
        this.audioFile = Fs.openSync(audioPath);
    }

    public startChargeSound(): void {
        console.info(LOGTAG, 'startChargeSound');
        if (this.isAudioPlaying) {
            console.warn(LOGTAG, 'ChargeSound is playing')
            return;
        }
        this.isAudioPlaying = true;
        this.playAudio();
    }

    protected async playAudio(): Promise<void> {
        console.info(LOGTAG, 'playAudio');
        // Create player
        let avPlayer: Media.AVPlayer = null;
        try {
            let audioRendererInfo: Audio.AudioRendererInfo = {
                content: Audio.ContentType.CONTENT_TYPE_UNKNOWN,
                usage: Audio.StreamUsage.STREAM_USAGE_SYSTEM,
                rendererFlags: 0
            }
            avPlayer = await Media.createAVPlayer();
            if (avPlayer == null) {
                console.error(LOGTAG, 'CreateAVPlayer failed');
                return;
            }
            console.info(LOGTAG, 'playAudio registe state');
            // Register state change listener
            this.avPlayerOnStateChange(avPlayer, audioRendererInfo);
            // Register audio interrupt listener
            this.avPlayerOnInterrupt(avPlayer);
            // Register error listener
            this.avPlayerOnError(avPlayer);
            // Register reset callback
            this.OnReset = (): void => {
                console.info(LOGTAG, 'OnReset called');
                avPlayer?.reset();
            }
        } catch (e) {
            console.error(LOGTAG, `playAudio CreateAVPlayer error: ${e?.message}`);
            this.Cleanup();
            return;
        }
        // start play
        try {
            avPlayer.url = 'fd://' + `${this.audioFile.fd}`; // Trigger initialized state
        } catch (e) {
            console.error(LOGTAG, `playAudio Start error: ${e?.message}`);
            avPlayer?.release();
            this.Cleanup();
            return;
        }
    }

    protected avPlayerOnStateChange(avPlayer: Media.AVPlayer, audioRendererInfo: Audio.AudioRendererInfo): void {
        avPlayer.on('stateChange', async (state: string, reason: Media.StateChangeReason) => {
            console.info(LOGTAG, 'avPlayerOnStateChange called, state: ' + state + ` reason: ${reason}`);
            switch (state) {
                case 'idle': // Successfully calling reset triggers idle state
                    avPlayer?.release();
                    break;
                case 'initialized': // Setting avplayer url successfully triggers initialized state
                    if (avPlayer) {
                        avPlayer.audioRendererInfo = audioRendererInfo;
                    }
                    avPlayer?.prepare();
                    break;
                case 'prepared': // Prepare execution successfully triggers prepared state
                    if (avPlayer) {
                        avPlayer.audioInterruptMode = Audio.InterruptMode.INDEPENDENT_MODE;
                    }
                    avPlayer?.play();
                    break;
                case 'playing': // Play execution successfully triggers playing state
                    break;
                case 'paused': // Pause execution successfully triggers paused state
                case 'completed': // Playback completion triggers completed state
                    avPlayer?.stop();    
                    break;
                case 'stopped': // Stop execution successfully triggers stopped state
                    avPlayer?.reset(); // Reset avPlayer state to idle
                    break;
                case 'released': // Release execution successfully triggers released state
                    this.Cleanup();
                    break;
                default:
                    console.warn(LOGTAG, 'avPlayer unknown state: ' + state);
                    break;
            }
        });
    }

    protected avPlayerOnInterrupt(avPlayer: Media.AVPlayer): void {
        avPlayer.on('audioInterrupt', (interruptEvent: Audio.InterruptEvent) => {
            console.info(LOGTAG, `avPlayerOnInterrupt called, ${JSON.stringify(interruptEvent)}`);
            try {
                avPlayer?.reset();
            } catch (e) {
                console.error(LOGTAG, `avPlayerOnInterrupt reset error: ${e?.message}`);
                this.Cleanup();
            }
        });
    }

    protected avPlayerOnError(avPlayer: Media.AVPlayer): void {
        avPlayer.on('error', (error: BusinessError) => {
            console.error(LOGTAG, `avPlayerOnError called, ${JSON.stringify(error)}`);
            try {
                avPlayer?.reset();
            } catch (e) {
                console.error(LOGTAG, `avPlayerOnError reset error: ${e?.message}`);
                this.Cleanup();
            }
        });
    }

    public stopChargeSound(): Promise<void> {
        console.info(LOGTAG, 'stopChargeSound isPlaying:', this.isAudioPlaying);
        
        // If audio is not playing, directly return resolved Promise
        if (!this.isAudioPlaying) {
            console.info(LOGTAG, 'Audio is not playing, return directly');
            return Promise.resolve();
        }
         // Set check interval (100ms) and maximum check count (20 times, total 2 seconds timeout)
        const checkInterval = 100; // Check interval in milliseconds
        const maxCheckCount = 20;  // Maximum check count (20 times, total 2 seconds timeout)
        let checkCount = 0;

        const stopPromise: Promise<void> = new Promise<void>((resolve) => {
            // Use setInterval to implement polling monitoring
            const intervalId = setInterval(() => {
                checkCount++;
                console.info(LOGTAG, `Checking audio status: isPlaying=${this.isAudioPlaying}, checkCount=${checkCount}`);
                // Check if monitoring should stop (audio stopped or timeout)
                if (!this.isAudioPlaying || checkCount >= maxCheckCount) {
                    clearInterval(intervalId);
                    
                    // Handle timeout case separately (without nesting)
                    if (checkCount >= maxCheckCount && this.isAudioPlaying) {
                        console.warn(LOGTAG, 'Audio playing timeout, forcing cleanup');
                        try {
                            // Attempt to stop play through callback
                            if (this.OnReset) {
                                this.OnReset();
                            }
                            // Directly call cleanup method to ensure resource release
                            this.Cleanup();
                        } catch (e) {
                            console.error(LOGTAG, `Forced cleanup error: ${e?.message}`);
                        }
                    }
                    
                    console.info(LOGTAG, 'stopChargeSound completed');
                    resolve();
                }
            }, checkInterval);
        });

        return stopPromise;
    }

    private Cleanup(): void {
        if (this.audioFile) {
            console.info(LOGTAG, 'released close audioFile');
            Fs.close(this.audioFile);
            this.audioFile = null;
        }
        this.isAudioPlaying = false;
    }
}
