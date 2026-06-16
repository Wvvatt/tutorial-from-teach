(function () {
  "use strict";

  function $(selector, root) {
    return (root || document).querySelector(selector);
  }

  function $all(selector, root) {
    return Array.prototype.slice.call((root || document).querySelectorAll(selector));
  }

  function logTo(el, message, type) {
    if (!el) return;
    var row = document.createElement("div");
    row.className = type || "info";
    row.textContent = "[" + new Date().toLocaleTimeString() + "] " + message;
    el.appendChild(row);
    el.scrollTop = el.scrollHeight;
  }

  function initQuizzes() {
    $all("[data-quiz]").forEach(function (quiz) {
      var submit = $("[data-quiz-submit]", quiz);
      var reset = $("[data-quiz-reset]", quiz);
      var score = $("[data-quiz-score]", quiz);

      function grade() {
        var total = 0;
        var correct = 0;
        $all("[data-answer]", quiz).forEach(function (q) {
          total += 1;
          var name = q.getAttribute("data-name");
          var answer = q.getAttribute("data-answer");
          var explanation = q.getAttribute("data-explain") || "";
          var selected = $("input[name=\"" + name + "\"]:checked", q);
          var feedback = $(".feedback", q);
          if (!feedback) return;

          if (!selected) {
            feedback.className = "feedback pending";
            feedback.textContent = "先选一个答案。";
            return;
          }

          if (selected.value === answer) {
            correct += 1;
            feedback.className = "feedback correct";
            feedback.textContent = "正确。" + explanation;
          } else {
            feedback.className = "feedback wrong";
            feedback.textContent = "不对。正确答案是 " + answer.toUpperCase() + "。" + explanation;
          }
        });

        if (score) {
          var pass = correct >= Math.ceil(total * 0.7);
          score.className = "quiz-score show " + (pass ? "pass" : "fail");
          score.textContent = "得分 " + correct + "/" + total + "。";
        }
      }

      function clear() {
        $all("input[type=\"radio\"]", quiz).forEach(function (input) {
          input.checked = false;
        });
        $all(".feedback", quiz).forEach(function (feedback) {
          feedback.className = "feedback";
          feedback.textContent = "";
        });
        if (score) {
          score.className = "quiz-score";
          score.textContent = "";
        }
      }

      if (submit) submit.addEventListener("click", grade);
      if (reset) reset.addEventListener("click", clear);
    });
  }

  function initMediaEventDemo() {
    var root = $("[data-media-event-demo]");
    if (!root) return;
    var videoBox = $(".video-surface", root);
    var log = $(".log", root);
    var status = $(".status", root);
    var meter = $(".meter span", root);
    var events = [
      ["loadstart", "网络层开始选择资源"],
      ["loadedmetadata", "容器头和轨道信息就绪"],
      ["canplay", "至少有当前帧和后续数据"],
      ["play", "HTMLMediaElement 接受播放请求"],
      ["waiting", "demux/decoder 暂时供帧不足"],
      ["playing", "缓冲恢复，渲染循环继续"],
      ["timeupdate", "currentTime 向前推进"],
      ["pause", "播放状态进入暂停"]
    ];
    var index = 0;
    var timer = null;

    function render() {
      var progress = Math.round((index / (events.length - 1)) * 100);
      if (meter) meter.style.width = progress + "%";
      if (status) status.textContent = "当前事件: " + events[index][0] + " | " + progress + "%";
      if (videoBox) {
        videoBox.textContent = events[index][0];
        videoBox.setAttribute("data-state", events[index][0]);
      }
    }

    function step() {
      var item = events[index];
      logTo(log, item[0] + " - " + item[1], item[0] === "waiting" ? "warn" : "info");
      render();
      if (index < events.length - 1) index += 1;
      else window.clearInterval(timer);
    }

    var start = $("[data-start]", root);
    var next = $("[data-next]", root);
    var reset = $("[data-reset]", root);
    if (start) {
      start.addEventListener("click", function () {
        window.clearInterval(timer);
        timer = window.setInterval(step, 700);
        step();
      });
    }
    if (next) {
      next.addEventListener("click", function () {
        window.clearInterval(timer);
        step();
      });
    }
    if (reset) {
      reset.addEventListener("click", function () {
        window.clearInterval(timer);
        index = 0;
        if (log) log.innerHTML = "";
        render();
      });
    }
    render();
  }

  function initRealVideoDemo() {
    var root = $("[data-real-video-demo]");
    if (!root) return;
    var video = $("video", root);
    var log = $(".log", root);
    var status = $(".status", root);
    var current = $("[data-video-current]", root);
    var duration = $("[data-video-duration]", root);
    var ready = $("[data-video-ready]", root);
    var network = $("[data-video-network]", root);
    var buffered = $("[data-video-buffered]", root);
    var playButton = $("[data-video-play]", root);
    var pauseButton = $("[data-video-pause]", root);
    var restartButton = $("[data-video-restart]", root);

    if (!video) return;

    var readyStates = ["HAVE_NOTHING", "HAVE_METADATA", "HAVE_CURRENT_DATA", "HAVE_FUTURE_DATA", "HAVE_ENOUGH_DATA"];
    var networkStates = ["NETWORK_EMPTY", "NETWORK_IDLE", "NETWORK_LOADING", "NETWORK_NO_SOURCE"];
    var interesting = [
      "loadstart", "durationchange", "loadedmetadata", "loadeddata", "canplay",
      "canplaythrough", "play", "playing", "pause", "waiting", "seeking",
      "seeked", "timeupdate", "ended", "error", "stalled", "suspend"
    ];

    function fmt(seconds) {
      if (!Number.isFinite(seconds)) return "--";
      return seconds.toFixed(2) + "s";
    }

    function bufferedText() {
      if (!video.buffered || video.buffered.length === 0) return "none";
      var ranges = [];
      for (var i = 0; i < video.buffered.length; i += 1) {
        ranges.push(fmt(video.buffered.start(i)) + "-" + fmt(video.buffered.end(i)));
      }
      return ranges.join(", ");
    }

    function updateStats(eventName) {
      if (current) current.textContent = fmt(video.currentTime);
      if (duration) duration.textContent = fmt(video.duration);
      if (ready) ready.textContent = readyStates[video.readyState] || String(video.readyState);
      if (network) network.textContent = networkStates[video.networkState] || String(video.networkState);
      if (buffered) buffered.textContent = bufferedText();
      if (status) status.textContent = (eventName || "ready") + " | paused=" + video.paused + " | playbackRate=" + video.playbackRate;
    }

    interesting.forEach(function (name) {
      video.addEventListener(name, function () {
        var type = name === "error" || name === "stalled" ? "err" : (name === "waiting" || name === "suspend" ? "warn" : "info");
        var extra = " t=" + fmt(video.currentTime) + " ready=" + (readyStates[video.readyState] || video.readyState);
        logTo(log, name + extra, type);
        updateStats(name);
      });
    });

    if (playButton) playButton.addEventListener("click", function () {
      var promise = video.play();
      if (promise && promise.catch) {
        promise.catch(function (err) {
          logTo(log, "play() 被浏览器策略拒绝: " + err.message, "warn");
        });
      }
    });
    if (pauseButton) pauseButton.addEventListener("click", function () { video.pause(); });
    if (restartButton) restartButton.addEventListener("click", function () {
      video.currentTime = 0;
      video.play();
    });

    updateStats("ready");
  }

  function initDashDrmDemo() {
    var root = $("[data-dash-drm-demo]");
    if (!root) return;
    var video = $("video", root);
    var log = $(".log", root);
    var status = $(".status", root);
    var clearButton = $("[data-load-clear]", root);
    var encryptedButton = $("[data-load-encrypted]", root);
    var stopButton = $("[data-stop]", root);
    var playButton = $("[data-play]", root);
    var sourceLabel = $("[data-source-label]", root);
    var player = null;
    var clearMpd = root.getAttribute("data-clear-mpd");
    var encryptedMpd = root.getAttribute("data-encrypted-mpd");
    var licenseServer = root.getAttribute("data-license-server");

    if (!video) return;

    function setStatus(text) {
      if (status) status.textContent = text;
    }

    function destroyPlayer() {
      if (player) {
        try {
          player.reset();
        } catch (err) {
          logTo(log, "dash.js reset 失败: " + err.message, "warn");
        }
        player = null;
      }
      video.removeAttribute("src");
      video.load();
    }

    function attachEvents(instance) {
      var events = window.dashjs.MediaPlayer.events;
      var eventNames = [
        "MANIFEST_LOADED", "STREAM_INITIALIZED", "PLAYBACK_STARTED",
        "PLAYBACK_PAUSED", "PLAYBACK_WAITING", "PLAYBACK_PLAYING",
        "PLAYBACK_ENDED", "QUALITY_CHANGE_RENDERED", "FRAGMENT_LOADING_COMPLETED",
        "ERROR", "KEY_SESSION_CREATED", "KEY_SESSION_UPDATED",
        "KEY_STATUSES_CHANGED", "LICENSE_REQUEST_COMPLETE"
      ];

      eventNames.forEach(function (eventName) {
        if (!events[eventName]) return;
        instance.on(events[eventName], function (event) {
          var type = eventName === "ERROR" ? "err" : (eventName.indexOf("KEY") === 0 || eventName.indexOf("LICENSE") === 0 ? "ok" : "info");
          var detail = "";
          if (eventName === "ERROR") {
            detail = " " + (event && event.error ? event.error : JSON.stringify(event));
          } else if (event && event.mediaType) {
            detail = " " + event.mediaType;
          }
          logTo(log, eventName + detail, type);
        });
      });
    }

    function load(kind) {
      if (!window.dashjs || !window.dashjs.MediaPlayer) {
        setStatus("dash.js 未加载。确认 assets/vendor/dash.all.min.js 存在。");
        logTo(log, "dash.js 不存在，无法播放 DASH。", "err");
        return;
      }

      destroyPlayer();
      if (log) log.innerHTML = "";

      var isEncrypted = kind === "encrypted";
      var mpd = isEncrypted ? encryptedMpd : clearMpd;
      if (sourceLabel) sourceLabel.textContent = (isEncrypted ? "Encrypted CENC: " : "Clear: ") + mpd;
      setStatus("加载 " + (isEncrypted ? "Widevine CENC" : "clear") + " DASH...");

      player = window.dashjs.MediaPlayer().create();
      attachEvents(player);

      player.updateSettings({
        streaming: {
          buffer: { fastSwitchEnabled: true },
          abr: { autoSwitchBitrate: { video: true, audio: true } }
        }
      });

      if (isEncrypted) {
        player.setProtectionData({
          "com.widevine.alpha": {
            serverURL: licenseServer
          }
        });
        logTo(log, "Widevine license server: " + licenseServer, "info");
      }

      player.initialize(video, mpd, false);
      logTo(log, "MPD: " + mpd, "info");
      setStatus((isEncrypted ? "加密流" : "清流") + " 已加载，点击播放。");
    }

    if (clearButton) clearButton.addEventListener("click", function () { load("clear"); });
    if (encryptedButton) encryptedButton.addEventListener("click", function () { load("encrypted"); });
    if (stopButton) stopButton.addEventListener("click", function () {
      destroyPlayer();
      setStatus("已停止。");
      if (sourceLabel) sourceLabel.textContent = "未加载";
      logTo(log, "player stopped", "info");
    });
    if (playButton) playButton.addEventListener("click", function () {
      var promise = video.play();
      if (promise && promise.catch) {
        promise.catch(function (err) {
          logTo(log, "play() 失败: " + err.message, "warn");
        });
      }
    });

    logTo(log, "播放器就绪。先加载 clear 流确认 MSE/DASH，再加载 Widevine CENC 流确认 EME/CDM。", "info");
  }

  function initBlinkBridgeDemo() {
    var root = $("[data-blink-bridge-demo]");
    if (!root) return;
    var nodes = $all(".node", root);
    var log = $(".log", root);
    var status = $(".status", root);
    var stepIndex = 0;
    var steps = [
      ["HTMLVideoElement", "DOM 节点被解析，HTMLVideoElement 创建。"],
      ["HTMLMediaElement", "通用媒体状态机接管 load/play/seek。"],
      ["WebMediaPlayer", "Blink 调用平台抽象接口。"],
      ["WebMediaPlayerImpl", "选择 progressive 或 MSE 路径并创建 pipeline。"],
      ["media::Pipeline", "demuxer、decoder、renderer 初始化。"],
      ["Compositor", "VideoFrameProvider 把帧交给 cc/viz。"]
    ];

    function paint() {
      nodes.forEach(function (n) {
        n.classList.toggle("active", n.getAttribute("data-node") === steps[stepIndex][0]);
      });
      if (status) status.textContent = steps[stepIndex][0] + " - " + steps[stepIndex][1];
    }

    var advance = $("[data-advance]", root);
    var reset = $("[data-reset]", root);
    if (advance) {
      advance.addEventListener("click", function () {
        logTo(log, steps[stepIndex][1], "info");
        stepIndex = Math.min(stepIndex + 1, steps.length - 1);
        paint();
      });
    }
    if (reset) {
      reset.addEventListener("click", function () {
        stepIndex = 0;
        if (log) log.innerHTML = "";
        paint();
      });
    }
    paint();
  }

  function initMseFrameDemo() {
    var root = $("[data-mse-frame-demo]");
    if (!root) return;
    var track = $(".frame-track", root);
    var log = $(".log", root);
    var state = { frames: [], hasInit: false, hasKey: false, pts: 0 };

    function pill(frame) {
      var el = document.createElement("span");
      el.className = "frame-pill frame-" + frame.kind;
      el.textContent = frame.label;
      el.title = frame.title || "";
      return el;
    }

    function render() {
      track.innerHTML = "";
      if (!state.frames.length) {
        var empty = document.createElement("span");
        empty.className = "chip";
        empty.textContent = "track buffer 为空";
        track.appendChild(empty);
        return;
      }
      state.frames.forEach(function (frame) {
        track.appendChild(pill(frame));
      });
    }

    function append(kind) {
      var dropOrphan = $("[data-drop-orphan]", root);
      if (kind === "init") {
        if (state.hasInit) {
          logTo(log, "init segment 已存在，重复 append 会被忽略。", "warn");
          return;
        }
        state.hasInit = true;
        state.frames.push({ kind: "init", label: "init", title: "ftyp + moov，建立轨道配置" });
        logTo(log, "解析 init segment，SourceBufferStream 进入可接收 media segment 状态。", "ok");
      } else if (!state.hasInit) {
        logTo(log, "必须先 append init segment。", "err");
        return;
      } else if (kind === "key") {
        state.hasKey = true;
        state.frames.push({ kind: "key", label: "I " + state.pts, title: "关键帧，解码依赖起点" });
        logTo(log, "追加关键帧 I@" + state.pts + "。", "ok");
        state.pts += 1;
      } else if (kind === "delta") {
        if (!state.hasKey && dropOrphan && dropOrphan.checked) {
          state.frames.push({ kind: "drop", label: "drop P" + state.pts, title: "缺少关键帧依赖，被丢弃" });
          logTo(log, "delta 帧缺少关键帧依赖，被标记为 orphan。", "warn");
          state.pts += 1;
          render();
          return;
        }
        var type = state.pts % 3 === 0 ? "B" : "P";
        state.frames.push({ kind: "delta", label: type + " " + state.pts, title: "delta 帧，需要前向依赖" });
        logTo(log, "追加 delta 帧 " + type + "@" + state.pts + "。", "info");
        state.pts += 1;
      }
      render();
    }

    function decode() {
      var decoded = 0;
      state.frames.forEach(function (frame) {
        if (frame.kind === "key" || frame.kind === "delta") {
          frame.kind = "out";
          frame.label = "out " + frame.label;
          decoded += 1;
        }
      });
      logTo(log, "decoder 拉取 " + decoded + " 帧；输出顺序需要按 DTS，显示顺序按 PTS。", decoded ? "ok" : "warn");
      render();
    }

    function clear() {
      state.frames = [];
      state.hasInit = false;
      state.hasKey = false;
      state.pts = 0;
      if (log) log.innerHTML = "";
      logTo(log, "track buffer 已清空。", "info");
      render();
    }

    $all("[data-append]", root).forEach(function (button) {
      button.addEventListener("click", function () {
        append(button.getAttribute("data-append"));
      });
    });
    var decodeButton = $("[data-decode]", root);
    var clearButton = $("[data-clear]", root);
    if (decodeButton) decodeButton.addEventListener("click", decode);
    if (clearButton) clearButton.addEventListener("click", clear);
    render();
  }

  function initCapabilityDemo() {
    var root = $("[data-capability-demo]");
    if (!root) return;
    var grid = $(".cap-grid", root);
    var log = $(".log", root);
    var mode = root.getAttribute("data-capability-demo");

    function card(name, state, detail) {
      var div = document.createElement("div");
      div.className = "cap-card " + state;
      div.innerHTML = "<strong>" + name + "</strong><small>" + detail + "</small>";
      return div;
    }

    async function run() {
      grid.innerHTML = "";
      if (log) log.innerHTML = "";
      var video = document.createElement("video");
      var codecs = [
        ["H.264 Baseline", "video/mp4; codecs=\"avc1.42E01E\""],
        ["H.264 High", "video/mp4; codecs=\"avc1.640028\""],
        ["HEVC", "video/mp4; codecs=\"hev1.1.6.L120.90\""],
        ["VP9", "video/webm; codecs=\"vp9\""],
        ["AV1", "video/mp4; codecs=\"av01.0.05M.08\""]
      ];

      codecs.forEach(function (item) {
        var answer = video.canPlayType(item[1]);
        var ok = answer === "probably" || answer === "maybe";
        grid.appendChild(card(item[0], ok ? "yes" : "no", answer || "canPlayType 返回空"));
        logTo(log, item[0] + ": " + (answer || "no"), ok ? "ok" : "warn");
      });

      if ("mediaCapabilities" in navigator && navigator.mediaCapabilities.decodingInfo) {
        try {
          var info = await navigator.mediaCapabilities.decodingInfo({
            type: "file",
            video: {
              contentType: "video/mp4; codecs=\"avc1.640028\"",
              width: 1920,
              height: 1080,
              bitrate: 8000000,
              framerate: 30
            }
          });
          grid.appendChild(card("MediaCapabilities 1080p H.264", info.supported ? "yes" : "no", "smooth=" + info.smooth + ", powerEfficient=" + info.powerEfficient));
          logTo(log, "MediaCapabilities: " + JSON.stringify(info), info.supported ? "ok" : "warn");
        } catch (err) {
          grid.appendChild(card("MediaCapabilities", "unknown", err.message));
          logTo(log, "MediaCapabilities 查询失败: " + err.message, "warn");
        }
      } else {
        grid.appendChild(card("MediaCapabilities", "unknown", "当前浏览器未暴露 API"));
      }

      var drmSystems = mode === "platform" ? ["org.w3.clearkey", "com.widevine.alpha", "com.microsoft.playready"] : ["org.w3.clearkey", "com.widevine.alpha"];
      if (navigator.requestMediaKeySystemAccess) {
        for (var i = 0; i < drmSystems.length; i += 1) {
          var keySystem = drmSystems[i];
          try {
            await navigator.requestMediaKeySystemAccess(keySystem, [{
              initDataTypes: ["cenc"],
              videoCapabilities: [{ contentType: "video/mp4; codecs=\"avc1.42E01E\"" }]
            }]);
            grid.appendChild(card(keySystem, "yes", "requestMediaKeySystemAccess 成功"));
            logTo(log, keySystem + " 可用。", "ok");
          } catch (err2) {
            grid.appendChild(card(keySystem, "no", "不可用或策略拒绝"));
            logTo(log, keySystem + " 不可用。", "warn");
          }
        }
      } else {
        grid.appendChild(card("EME", "unknown", "未提供 requestMediaKeySystemAccess"));
      }

      if (mode === "platform") {
        var webgl = false;
        try { webgl = !!document.createElement("canvas").getContext("webgl"); } catch (e) {}
        grid.appendChild(card("WebGL", webgl ? "yes" : "no", "用于 GPU 合成可见性检测"));
        grid.appendChild(card("WebCodecs", "VideoDecoder" in window ? "yes" : "no", "浏览器是否暴露底层编解码 API"));
      }
    }

    var button = $("[data-run-capability]", root);
    if (button) button.addEventListener("click", run);
    run();
  }

  function initOverlayCanvas() {
    var canvas = $("[data-overlay-canvas]");
    if (!canvas) return;
    var ctx = canvas.getContext("2d");
    var root = canvas.closest(".demo");
    var mode = "overlay";
    var hints = false;
    var frame = 0;
    var label = $("[data-frame-label]", root);

    function line(x1, y1, x2, y2, color) {
      ctx.strokeStyle = color;
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(x1, y1);
      ctx.lineTo(x2, y2);
      ctx.stroke();
      var angle = Math.atan2(y2 - y1, x2 - x1);
      ctx.beginPath();
      ctx.moveTo(x2, y2);
      ctx.lineTo(x2 - 9 * Math.cos(angle - 0.45), y2 - 9 * Math.sin(angle - 0.45));
      ctx.lineTo(x2 - 9 * Math.cos(angle + 0.45), y2 - 9 * Math.sin(angle + 0.45));
      ctx.closePath();
      ctx.fillStyle = color;
      ctx.fill();
    }

    function box(x, y, w, h, color, text, ink) {
      ctx.fillStyle = color;
      ctx.fillRect(x, y, w, h);
      ctx.fillStyle = ink || "#fff";
      ctx.font = "14px sans-serif";
      ctx.textAlign = "center";
      ctx.fillText(text, x + w / 2, y + h / 2 + 5);
    }

    function draw() {
      var w = canvas.width;
      var h = canvas.height;
      ctx.clearRect(0, 0, w, h);
      ctx.fillStyle = "#111827";
      ctx.fillRect(0, 0, w, h);
      ctx.strokeStyle = "#283142";
      for (var x = 0; x < w; x += 44) {
        ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, h); ctx.stroke();
      }
      for (var y = 0; y < h; y += 44) {
        ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
      }

      var t = Date.now() / 1000;
      var px = 90 + ((t * 78) % 420);
      box(px, 50, 92, 30, "#0b6bcb", "VideoFrame");
      line(px + 92, 65, px + 145, 125, "#38bdf8");
      box(px + 135, 110, 130, 34, "#0f766e", "Compositor");
      line(px + 265, 127, px + 318, 197, mode === "overlay" ? "#22c55e" : "#f97316");
      box(px + 300, 185, 150, 38, mode === "overlay" ? "#15803d" : "#b45309", mode === "overlay" ? "Overlay promote" : "GPU blend");
      line(px + 450, 204, Math.min(px + 520, w - 80), 285, mode === "overlay" ? "#22c55e" : "#f97316");
      box(Math.min(px + 500, w - 160), 270, 110, 34, "#334155", "Screen");

      if (hints) {
        ctx.setLineDash([6, 5]);
        ctx.strokeStyle = "#facc15";
        ctx.strokeRect(px + 105, 178, 100, 28);
        ctx.setLineDash([]);
        ctx.fillStyle = "#facc15";
        ctx.font = "13px sans-serif";
        ctx.fillText("hint frame", px + 155, 174);
      }

      var latency = mode === "overlay" ? 8 + Math.round(Math.sin(t * 3) * 2) : 32 + Math.round(Math.sin(t * 2) * 8);
      if (label) label.textContent = "帧 #" + frame + " | 估计延迟 " + latency + "ms | " + (mode === "overlay" ? "低功耗" : "高灵活性");
      frame += 1;
      window.requestAnimationFrame(draw);
    }

    var overlay = $("[data-mode-overlay]", root);
    var composite = $("[data-mode-composite]", root);
    var hintButton = $("[data-toggle-hints]", root);
    if (overlay) overlay.addEventListener("click", function () {
      mode = "overlay";
      overlay.classList.remove("secondary");
      if (composite) composite.classList.add("secondary");
    });
    if (composite) composite.addEventListener("click", function () {
      mode = "composite";
      composite.classList.remove("secondary");
      if (overlay) overlay.classList.add("secondary");
    });
    if (hintButton) hintButton.addEventListener("click", function () {
      hints = !hints;
      hintButton.textContent = hints ? "隐藏 hint" : "显示 hint";
    });
    draw();
  }

  function initPlaneBudgetDemo() {
    var root = $("[data-plane-budget-demo]");
    if (!root) return;
    var overlayCount = $("[data-overlay-count]", root);
    var videoCount = $("[data-video-count]", root);
    var output = $(".readout", root);

    function update() {
      var planes = Number(overlayCount.value);
      var videos = Number(videoCount.value);
      var promoted = Math.min(planes, videos);
      var composited = Math.max(0, videos - planes);
      output.innerHTML = [
        "overlay planes: " + planes,
        "video layers: " + videos,
        "promoted: " + promoted,
        "GPU composited fallback: " + composited,
        composited ? "结论: 至少 " + composited + " 路视频会增加 GPU 带宽和功耗。" : "结论: 当前视频层都可尝试走 overlay。"
      ].join("<br>");
    }

    if (overlayCount) overlayCount.addEventListener("input", update);
    if (videoCount) videoCount.addEventListener("input", update);
    update();
  }

  function initDrmSessionDemo() {
    var root = $("[data-drm-session-demo]");
    if (!root) return;
    var nodes = $all(".node", root);
    var log = $(".log", root);
    var status = $(".status", root);
    var index = 0;
    var steps = [
      ["Initial", "createSession() 只创建会话，不产生 license challenge。"],
      ["Pending", "generateRequest(initData) 后 CDM 生成 challenge。"],
      ["License", "网页把 challenge 送到 license server。"],
      ["Usable", "session.update(license) 后 keyStatuses 变为 usable。"],
      ["Secure output", "L1 需要 secure decode、secure overlay 和 HDCP。"]
    ];

    function paint() {
      nodes.forEach(function (n) {
        n.classList.toggle("active", n.getAttribute("data-node") === steps[index][0]);
      });
      if (status) status.textContent = steps[index][0] + " - " + steps[index][1];
    }

    var next = $("[data-next]", root);
    var reset = $("[data-reset]", root);
    if (next) next.addEventListener("click", function () {
      logTo(log, steps[index][1], index === 4 ? "ok" : "info");
      index = Math.min(index + 1, steps.length - 1);
      paint();
    });
    if (reset) reset.addEventListener("click", function () {
      index = 0;
      if (log) log.innerHTML = "";
      paint();
    });
    paint();
  }

  document.addEventListener("DOMContentLoaded", function () {
    initQuizzes();
    initRealVideoDemo();
    initDashDrmDemo();
    initMediaEventDemo();
    initBlinkBridgeDemo();
    initMseFrameDemo();
    initCapabilityDemo();
    initOverlayCanvas();
    initPlaneBudgetDemo();
    initDrmSessionDemo();
  });
}());
