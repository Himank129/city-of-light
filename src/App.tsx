/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import React, { useEffect, useRef, useState } from 'react';
import * as THREE from 'three';
import JSZip from 'jszip';
import {
  Monitor,
  Terminal,
  FolderTree,
  FileCode2,
  Download,
  Copy,
  Check,
  Play,
  RotateCw,
  Cpu,
  Layers,
  Sparkles,
  Maximize2,
  Eye,
  Shield,
  Gauge,
  ExternalLink,
} from 'lucide-react';
import { NATIVE_FILES, NativeFile } from './nativeSources';

export default function App() {
  const mountRef = useRef<HTMLDivElement>(null);
  const [selectedFile, setSelectedFile] = useState<NativeFile>(
    NATIVE_FILES.find((f) => f.path === 'src/Main.cpp') || NATIVE_FILES[0]
  );
  const [copiedFile, setCopiedFile] = useState<string | null>(null);
  const [isZipping, setIsZipping] = useState(false);
  const [zipSuccess, setZipSuccess] = useState(false);
  const [activeTab, setActiveTab] = useState<'viewport' | 'code' | 'architecture' | 'build'>('viewport');
  
  // Viewport telemetry state
  const [fps, setFps] = useState(60);
  const [camPos, setCamPos] = useState({ x: 0, y: 3.5, z: -18 });
  const [isWireframe, setIsWireframe] = useState(false);
  const [vsync, setVsync] = useState(true);
  const [cameraMode, setCameraMode] = useState<'fps' | 'orbit'>('fps');
  const [isMouseLocked, setIsMouseLocked] = useState(false);

  // References to communicate with Three.js game loop
  const wireframeRef = useRef(false);
  wireframeRef.current = isWireframe;

  // --------------------------------------------------------------------------
  // Three.js Interactive Engine Viewport (Mirror of DirectX 11 Main.cpp Scene)
  // --------------------------------------------------------------------------
  useEffect(() => {
    if (!mountRef.current) return;
    const container = mountRef.current;
    const width = container.clientWidth;
    const height = container.clientHeight;

    // 1. Scene & Signature Midnight Navy Fog (matching Basic3D.hlsl)
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0x0a0f1c);
    scene.fog = new THREE.FogExp2(0x0a0f1c, 0.007);

    // 2. Camera setup matching Camera.cpp (65 deg FOV, near 0.1, far 1000)
    const camera = new THREE.PerspectiveCamera(65, width / height, 0.1, 1000);
    camera.position.set(0, 3.5, -18);
    camera.lookAt(0, 4, 0);

    // 3. Renderer
    const renderer = new THREE.WebGLRenderer({ antialias: true, powerPreference: 'high-performance' });
    renderer.setSize(width, height);
    renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFShadowMap;
    container.appendChild(renderer.domElement);

    // 4. Lighting matching Basic3D.hlsl
    // Warm California directional sunlight
    const sunLight = new THREE.DirectionalLight(0xfff5e6, 1.8);
    sunLight.position.set(-40, 80, -45);
    sunLight.castShadow = true;
    sunLight.shadow.mapSize.width = 2048;
    sunLight.shadow.mapSize.height = 2048;
    sunLight.shadow.camera.near = 10;
    sunLight.shadow.camera.far = 250;
    const d = 60;
    sunLight.shadow.camera.left = -d;
    sunLight.shadow.camera.right = d;
    sunLight.shadow.camera.top = d;
    sunLight.shadow.camera.bottom = -d;
    scene.add(sunLight);

    // Ambient hemisphere gradient (Sky cyan / Ground dark slate)
    const hemiLight = new THREE.HemisphereLight(0x1a2e4a, 0x06080d, 0.7);
    scene.add(hemiLight);

    // 5. Build 3D Scene Geometry (Matching Geometry.hpp & Main.cpp)
    // 5A. 300m x 300m Asphalt Grid with Neon Cyan Markings
    const gridGroup = new THREE.Group();
    const groundGeo = new THREE.PlaneGeometry(300, 300, 60, 60);
    const groundMat = new THREE.MeshStandardMaterial({
      color: 0x1a1e28,
      roughness: 0.85,
      metalness: 0.15,
    });
    const groundMesh = new THREE.Mesh(groundGeo, groundMat);
    groundMesh.rotation.x = -Math.PI / 2;
    groundMesh.receiveShadow = true;
    gridGroup.add(groundMesh);

    // Neon road grid overlay
    const gridHelper = new THREE.GridHelper(300, 30, 0x00f0ff, 0x223048);
    gridHelper.position.y = 0.05;
    gridGroup.add(gridHelper);
    scene.add(gridGroup);

    // 5B. Financial District Skyscraper Monoliths (6m x 32m x 6m)
    const buildingGeo = new THREE.BoxGeometry(6, 32, 6);
    const buildingMat = new THREE.MeshStandardMaterial({
      color: 0x121722,
      roughness: 0.35,
      metalness: 0.75,
    });

    const buildingPositions = [
      [-30, 16, 40],
      [30, 16, 40],
      [-30, 16, -40],
      [30, 16, -40],
    ];

    const buildings: THREE.Mesh[] = [];
    buildingPositions.forEach(([x, y, z]) => {
      const bld = new THREE.Mesh(buildingGeo, buildingMat);
      bld.position.set(x, y, z);
      bld.castShadow = true;
      bld.receiveShadow = true;
      scene.add(bld);
      buildings.push(bld);

      // Neon roof beacon
      const beaconGeo = new THREE.BoxGeometry(5.8, 0.6, 5.8);
      const beaconMat = new THREE.MeshBasicMaterial({ color: 0x00f0ff });
      const beacon = new THREE.Mesh(beaconGeo, beaconMat);
      beacon.position.set(x, 32.2, z);
      scene.add(beacon);
    });

    // 5C. Central Rotating Cyber Monelisk (DirectX 11 Demo Object)
    const coreGroup = new THREE.Group();
    coreGroup.position.set(0, 4, 0);

    // Outer neon wireframe cube
    const outerGeo = new THREE.BoxGeometry(3.2, 3.2, 3.2);
    const outerMat = new THREE.MeshStandardMaterial({
      color: 0x040812,
      roughness: 0.1,
      metalness: 0.9,
      emissive: 0x00f0ff,
      emissiveIntensity: 0.6,
    });
    const outerCube = new THREE.Mesh(outerGeo, outerMat);
    outerCube.castShadow = true;
    coreGroup.add(outerCube);

    // Inner glowing crystal core
    const innerGeo = new THREE.OctahedronGeometry(1.6);
    const innerMat = new THREE.MeshBasicMaterial({ color: 0x00f0ff, wireframe: true });
    const innerCore = new THREE.Mesh(innerGeo, innerMat);
    coreGroup.add(innerCore);

    // Ambient cyber point light
    const coreLight = new THREE.PointLight(0x00f0ff, 4, 30);
    coreGroup.add(coreLight);

    scene.add(coreGroup);

    // 6. Camera Controls (WASD & Mouse Look)
    const keys: { [key: string]: boolean } = {};
    const onKeyDown = (e: KeyboardEvent) => {
      keys[e.key.toLowerCase()] = true;
      if (e.key === 'F1') {
        e.preventDefault();
        setIsWireframe((prev) => !prev);
      }
      if (e.key.toLowerCase() === 'v') {
        setVsync((prev) => !prev);
      }
    };
    const onKeyUp = (e: KeyboardEvent) => {
      keys[e.key.toLowerCase()] = false;
    };

    window.addEventListener('keydown', onKeyDown);
    window.addEventListener('keyup', onKeyUp);

    // Mouse look state
    let isDragging = false;
    let previousMousePosition = { x: 0, y: 0 };
    let yaw = 0;
    let pitch = 0;

    const onMouseDown = (e: MouseEvent) => {
      isDragging = true;
      previousMousePosition = { x: e.clientX, y: e.clientY };
    };

    const onMouseMove = (e: MouseEvent) => {
      if (!isDragging) return;
      const deltaX = e.clientX - previousMousePosition.x;
      const deltaY = e.clientY - previousMousePosition.y;
      previousMousePosition = { x: e.clientX, y: e.clientY };

      const sensitivity = 0.003;
      yaw -= deltaX * sensitivity;
      pitch -= deltaY * sensitivity;
      pitch = Math.max(-1.45, Math.min(1.45, pitch)); // clamp [-85, 85] deg
    };

    const onMouseUp = () => {
      isDragging = false;
    };

    container.addEventListener('mousedown', onMouseDown);
    window.addEventListener('mousemove', onMouseMove);
    window.addEventListener('mouseup', onMouseUp);

    // Resize Handler
    const onResize = () => {
      if (!mountRef.current) return;
      const w = mountRef.current.clientWidth;
      const h = mountRef.current.clientHeight;
      camera.aspect = w / h;
      camera.updateProjectionMatrix();
      renderer.setSize(w, h);
    };
    window.addEventListener('resize', onResize);

    // 7. Engine Game Loop
    let lastTime = performance.now();
    let frameCount = 0;
    let fpsAccumulator = 0;
    let animId = 0;

    const animate = (now: number) => {
      animId = requestAnimationFrame(animate);
      const dt = Math.min((now - lastTime) / 1000, 0.1);
      lastTime = now;

      // FPS Calculation
      frameCount++;
      fpsAccumulator += dt;
      if (fpsAccumulator >= 0.5) {
        setFps(Math.round(frameCount / fpsAccumulator));
        frameCount = 0;
        fpsAccumulator = 0;
      }

      // Movement Kinematics
      const speed = keys['shift'] ? 30.0 : 12.0;
      const forward = new THREE.Vector3(-Math.sin(yaw), 0, Math.cos(yaw)).normalize();
      const right = new THREE.Vector3(Math.cos(yaw), 0, Math.sin(yaw)).normalize();

      if (keys['w']) camera.position.addScaledVector(forward, speed * dt);
      if (keys['s']) camera.position.addScaledVector(forward, -speed * dt);
      if (keys['a']) camera.position.addScaledVector(right, -speed * dt);
      if (keys['d']) camera.position.addScaledVector(right, speed * dt);
      if (keys[' ']) camera.position.y += speed * dt;
      if (keys['c'] || keys['control']) camera.position.y -= speed * dt;

      // Update camera orientation (facing +Z toward city center at z=0)
      const targetLook = new THREE.Vector3(
        -Math.sin(yaw) * Math.cos(pitch),
        Math.sin(pitch),
        Math.cos(yaw) * Math.cos(pitch)
      );
      camera.lookAt(camera.position.clone().add(targetLook));

      // Rotate central cyber monolith
      coreGroup.rotation.y += 0.8 * dt;
      coreGroup.rotation.x += 0.4 * dt;
      innerCore.rotation.y -= 1.4 * dt;

      // Sync wireframe mode
      groundMat.wireframe = wireframeRef.current;
      buildingMat.wireframe = wireframeRef.current;
      outerMat.wireframe = wireframeRef.current;

      setCamPos({
        x: parseFloat(camera.position.x.toFixed(1)),
        y: parseFloat(camera.position.y.toFixed(1)),
        z: parseFloat(camera.position.z.toFixed(1)),
      });

      renderer.render(scene, camera);
    };

    animId = requestAnimationFrame(animate);

    return () => {
      cancelAnimationFrame(animId);
      window.removeEventListener('keydown', onKeyDown);
      window.removeEventListener('keyup', onKeyUp);
      container.removeEventListener('mousedown', onMouseDown);
      window.removeEventListener('mousemove', onMouseMove);
      window.removeEventListener('mouseup', onMouseUp);
      window.removeEventListener('resize', onResize);
      renderer.dispose();
      if (container.contains(renderer.domElement)) {
        container.removeChild(renderer.domElement);
      }
    };
  }, []);

  // --------------------------------------------------------------------------
  // Copy to Clipboard
  // --------------------------------------------------------------------------
  const copyFileContent = (content: string, path: string) => {
    navigator.clipboard.writeText(content);
    setCopiedFile(path);
    setTimeout(() => setCopiedFile(null), 2000);
  };

  // --------------------------------------------------------------------------
  // One-Click Native Source Package Generator (.ZIP)
  // --------------------------------------------------------------------------
  const downloadSourceZip = async () => {
    setIsZipping(true);
    try {
      const zip = new JSZip();
      const rootFolder = zip.folder('neon-city-native');

      NATIVE_FILES.forEach((file) => {
        rootFolder?.file(file.path, file.content);
      });

      const blob = await zip.generateAsync({ type: 'blob' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = 'Project_Neon_City_Native_Stage2.zip';
      document.body.appendChild(a);
      a.click();
      document.body.removeChild(a);
      URL.revokeObjectURL(url);

      setZipSuccess(true);
      setTimeout(() => setZipSuccess(false), 3000);
    } catch (err) {
      console.error('Failed to generate zip:', err);
    } finally {
      setIsZipping(false);
    }
  };

  return (
    <div className="flex flex-col h-screen w-screen bg-slate-950 text-slate-100 font-mono select-none overflow-hidden">
      {/* Top Header Navigation Strip */}
      <header className="flex items-center justify-between px-4 py-2.5 bg-slate-900 border-b border-cyan-500/30 shrink-0">
        <div className="flex items-center gap-3">
          <div className="w-3 h-3 bg-cyan-400 rotate-45 shadow-[0_0_10px_#00f0ff] animate-pulse" />
          <div>
            <h1 className="text-sm font-bold tracking-wider text-white flex items-center gap-2">
              <span>PROJECT NEON CITY</span>
              <span className="text-xs px-2 py-0.5 bg-cyan-950/80 text-cyan-300 border border-cyan-700/60 font-semibold">
                STAGE 2: NATIVE C++20 & DIRECTX 11
              </span>
            </h1>
            <p className="text-[11px] text-slate-400">
              Win32 · DirectX 11.1 · HLSL 5.0 · RawInput · Target: AMD Ryzen 7 8845HS / Radeon 780M
            </p>
          </div>
        </div>

        {/* View Switching Tabs */}
        <div className="flex items-center gap-1 bg-slate-950/80 p-1 border border-slate-800">
          <button
            onClick={() => setActiveTab('viewport')}
            className={`px-3 py-1 text-xs font-semibold flex items-center gap-1.5 transition-colors cursor-pointer ${
              activeTab === 'viewport' ? 'bg-cyan-500 text-slate-950' : 'text-slate-300 hover:text-white'
            }`}
          >
            <Monitor className="w-3.5 h-3.5" />
            <span>3D Viewport</span>
          </button>
          <button
            onClick={() => setActiveTab('code')}
            className={`px-3 py-1 text-xs font-semibold flex items-center gap-1.5 transition-colors cursor-pointer ${
              activeTab === 'code' ? 'bg-cyan-500 text-slate-950' : 'text-slate-300 hover:text-white'
            }`}
          >
            <FileCode2 className="w-3.5 h-3.5" />
            <span>Native C++ Code ({NATIVE_FILES.length})</span>
          </button>
          <button
            onClick={() => setActiveTab('architecture')}
            className={`px-3 py-1 text-xs font-semibold flex items-center gap-1.5 transition-colors cursor-pointer ${
              activeTab === 'architecture' ? 'bg-cyan-500 text-slate-950' : 'text-slate-300 hover:text-white'
            }`}
          >
            <Layers className="w-3.5 h-3.5" />
            <span>Subsystems Verification</span>
          </button>
          <button
            onClick={() => setActiveTab('build')}
            className={`px-3 py-1 text-xs font-semibold flex items-center gap-1.5 transition-colors cursor-pointer ${
              activeTab === 'build' ? 'bg-cyan-500 text-slate-950' : 'text-slate-300 hover:text-white'
            }`}
          >
            <Terminal className="w-3.5 h-3.5" />
            <span>Build & Run (.exe)</span>
          </button>
        </div>

        {/* One-Click Package Download */}
        <div className="flex items-center gap-2">
          <button
            onClick={downloadSourceZip}
            disabled={isZipping}
            className="flex items-center gap-2 px-3 py-1.5 bg-cyan-600 hover:bg-cyan-500 text-slate-950 font-bold text-xs tracking-wider border border-cyan-400 shadow-[0_0_12px_rgba(0,240,255,0.3)] transition-all cursor-pointer disabled:opacity-50"
            title="Download full C++20 / DirectX 11 Visual Studio project as a zip"
          >
            {zipSuccess ? (
              <>
                <Check className="w-3.5 h-3.5 text-slate-950" />
                <span>DOWNLOADED!</span>
              </>
            ) : (
              <>
                <Download className="w-3.5 h-3.5" />
                <span>{isZipping ? 'PACKING ZIP...' : 'DOWNLOAD NATIVE .ZIP'}</span>
              </>
            )}
          </button>
        </div>
      </header>

      {/* Main Body Area */}
      <div className="flex-1 relative overflow-hidden">
        {/* TAB 1: 3D VIEWPORT */}
        <div
          className={`absolute inset-0 flex flex-col ${
            activeTab === 'viewport' ? 'z-10 visible' : 'z-0 invisible pointer-events-none'
          }`}
        >
          {/* Top Telemetry & Controls Toolbar */}
          <div className="absolute top-3 left-3 z-20 flex items-center gap-2 pointer-events-auto">
            <div className="flex items-center gap-2 px-3 py-1 bg-slate-900/90 border border-cyan-500/40 text-xs backdrop-blur-md">
              <span className="text-cyan-400 font-bold">STAGE 2 VIEWPORT</span>
              <span className="text-slate-500">|</span>
              <span className="text-emerald-400 font-bold tabular-nums">{fps} FPS</span>
              <span className="text-slate-500">|</span>
              <span className="text-slate-300 tabular-nums">
                X:{camPos.x} Y:{camPos.y} Z:{camPos.z}
              </span>
            </div>

            <button
              onClick={() => setIsWireframe(!isWireframe)}
              className={`px-2.5 py-1 text-xs border font-semibold transition-colors cursor-pointer ${
                isWireframe
                  ? 'bg-amber-500/20 text-amber-300 border-amber-500'
                  : 'bg-slate-900/80 text-slate-300 border-slate-700 hover:border-slate-500'
              }`}
              title="Toggle Wireframe mode (F1 in native .exe)"
            >
              [F1] {isWireframe ? 'WIREFRAME' : 'SOLID PBR'}
            </button>

            <button
              onClick={() => setVsync(!vsync)}
              className={`px-2.5 py-1 text-xs border font-semibold transition-colors cursor-pointer ${
                vsync
                  ? 'bg-cyan-500/20 text-cyan-300 border-cyan-500'
                  : 'bg-slate-900/80 text-slate-400 border-slate-700'
              }`}
              title="Toggle VSync (V in native .exe)"
            >
              [V] {vsync ? 'VSYNC ON (60 FPS)' : 'VSYNC OFF'}
            </button>
          </div>

          {/* Bottom Controls Bar */}
          <div className="absolute bottom-3 left-3 right-3 z-20 flex items-center justify-between pointer-events-none">
            <div className="px-3 py-1.5 bg-slate-900/85 border border-slate-700/80 text-[11px] text-slate-300 flex items-center gap-3 backdrop-blur-sm pointer-events-auto">
              <span className="text-cyan-400 font-bold">CONTROLS:</span>
              <span>
                <kbd className="px-1 py-0.5 bg-slate-800 border border-slate-600 text-white">WASD</kbd> Move
              </span>
              <span>
                <kbd className="px-1 py-0.5 bg-slate-800 border border-slate-600 text-white">Shift</kbd> Sprint
              </span>
              <span>
                <kbd className="px-1 py-0.5 bg-slate-800 border border-slate-600 text-white">Space / C</kbd> Up / Down
              </span>
              <span>
                <kbd className="px-1 py-0.5 bg-slate-800 border border-slate-600 text-white">Click + Drag</kbd> Look
              </span>
            </div>

            <div className="px-3 py-1.5 bg-slate-900/85 border border-cyan-500/30 text-[11px] text-slate-300 backdrop-blur-sm">
              <span className="text-slate-400">Rendering Engine: </span>
              <span className="text-cyan-300 font-semibold">DirectX 11.1 / HLSL 5.0 Simulation</span>
            </div>
          </div>

          {/* Three.js Canvas Container */}
          <div ref={mountRef} className="w-full h-full cursor-crosshair" />
        </div>

        {/* TAB 2: NATIVE SOURCE CODE EXPLORER */}
        <div
          className={`absolute inset-0 flex bg-slate-950 ${
            activeTab === 'code' ? 'z-10 visible' : 'z-0 invisible pointer-events-none'
          }`}
        >
          {/* Sidebar File Tree */}
          <div className="w-72 bg-slate-900/90 border-r border-slate-800 flex flex-col shrink-0">
            <div className="p-3 border-b border-slate-800 bg-black/40 flex items-center justify-between">
              <div className="flex items-center gap-2 text-xs font-bold text-cyan-400">
                <FolderTree className="w-4 h-4" />
                <span>NEON-CITY-NATIVE/</span>
              </div>
              <span className="text-[10px] text-slate-400 px-1.5 py-0.5 bg-slate-800">
                {NATIVE_FILES.length} Files
              </span>
            </div>

            <div className="flex-1 overflow-y-auto py-2">
              {['Config', 'Shaders', 'Core', 'Platform', 'Renderer', 'Entry'].map((cat) => {
                const filesInCat = NATIVE_FILES.filter((f) => f.category === cat);
                if (filesInCat.length === 0) return null;

                return (
                  <div key={cat} className="mb-3">
                    <div className="px-3 py-1 text-[10px] font-bold text-slate-400 tracking-wider uppercase">
                      {cat}
                    </div>
                    {filesInCat.map((file) => {
                      const isSelected = selectedFile.path === file.path;
                      return (
                        <button
                          key={file.path}
                          onClick={() => setSelectedFile(file)}
                          className={`w-full text-left px-3 py-1.5 text-xs flex items-center justify-between transition-colors cursor-pointer ${
                            isSelected
                              ? 'bg-cyan-950 text-cyan-300 border-l-2 border-cyan-400 font-semibold'
                              : 'text-slate-400 hover:text-slate-200 hover:bg-slate-800/60'
                          }`}
                        >
                          <span className="truncate">{file.name}</span>
                          <span className="text-[10px] text-slate-500 uppercase font-mono ml-2">
                            {file.language}
                          </span>
                        </button>
                      );
                    })}
                  </div>
                );
              })}
            </div>
          </div>

          {/* Main Code Viewer */}
          <div className="flex-1 flex flex-col bg-slate-950 overflow-hidden">
            {/* File Path & Copy Toolbar */}
            <div className="flex items-center justify-between px-4 py-2.5 bg-slate-900 border-b border-slate-800">
              <div className="flex items-center gap-2">
                <FileCode2 className="w-4 h-4 text-cyan-400" />
                <span className="text-xs text-white font-bold">{selectedFile.path}</span>
                <span className="text-[10px] text-slate-400 px-2 py-0.5 bg-slate-800 border border-slate-700">
                  {selectedFile.language.toUpperCase()}
                </span>
              </div>

              <button
                onClick={() => copyFileContent(selectedFile.content, selectedFile.path)}
                className="flex items-center gap-1.5 px-3 py-1 text-xs bg-slate-800 hover:bg-slate-700 text-slate-200 border border-slate-600 transition-colors cursor-pointer"
              >
                {copiedFile === selectedFile.path ? (
                  <>
                    <Check className="w-3.5 h-3.5 text-emerald-400" />
                    <span className="text-emerald-400 font-semibold">COPIED</span>
                  </>
                ) : (
                  <>
                    <Copy className="w-3.5 h-3.5" />
                    <span>Copy Code</span>
                  </>
                )}
              </button>
            </div>

            {/* Code Content */}
            <div className="flex-1 overflow-auto p-4 bg-slate-950">
              <pre className="text-xs leading-relaxed font-mono text-slate-300 whitespace-pre">
                {selectedFile.content}
              </pre>
            </div>
          </div>
        </div>

        {/* TAB 3: SUBSYSTEMS VERIFICATION */}
        <div
          className={`absolute inset-0 overflow-y-auto p-6 bg-slate-950 ${
            activeTab === 'architecture' ? 'z-10 visible' : 'z-0 invisible pointer-events-none'
          }`}
        >
          <div className="max-w-5xl mx-auto space-y-6">
            <div>
              <h2 className="text-lg font-bold text-white flex items-center gap-2">
                <Cpu className="w-5 h-5 text-cyan-400" />
                <span>STAGE 2 SUBSYSTEMS VERIFICATION</span>
              </h2>
              <p className="text-xs text-slate-400">
                Detailed audit of the implemented native Windows C++20 and DirectX 11 foundational systems.
              </p>
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
              {/* Win32 Platform Layer */}
              <div className="p-4 bg-slate-900 border border-cyan-500/30">
                <div className="flex items-center justify-between mb-2">
                  <h3 className="text-sm font-bold text-white flex items-center gap-2">
                    <Monitor className="w-4 h-4 text-cyan-400" />
                    <span>Win32 Platform & Windowing</span>
                  </h3>
                  <span className="text-[10px] px-2 py-0.5 bg-emerald-950 text-emerald-400 border border-emerald-800 font-bold">
                    VERIFIED
                  </span>
                </div>
                <p className="text-xs text-slate-400 mb-3">
                  Located in <code className="text-cyan-300">src/Platform/Win32Window.cpp</code>
                </p>
                <ul className="text-xs text-slate-300 space-y-1.5 list-disc list-inside">
                  <li>
                    <strong className="text-white">WNDCLASSEXW Registration</strong>: Handles clean creation and teardown.
                  </li>
                  <li>
                    <strong className="text-white">Per-Monitor V2 DPI Awareness</strong>: Invokes{' '}
                    <code>SetProcessDpiAwarenessContext</code> for crisp rendering on 1440p / 4K displays.
                  </li>
                  <li>
                    <strong className="text-white">WM_SIZE SwapChain Hook</strong>: Dynamically resizes back buffer and depth stencil views without resource leaks.
                  </li>
                  <li>
                    <strong className="text-white">RawInput (WM_INPUT)</strong>: Captures non-accelerated sub-pixel mouse deltas for tactical FPS look.
                  </li>
                </ul>
              </div>

              {/* DirectX 11 Pipeline */}
              <div className="p-4 bg-slate-900 border border-cyan-500/30">
                <div className="flex items-center justify-between mb-2">
                  <h3 className="text-sm font-bold text-white flex items-center gap-2">
                    <Layers className="w-4 h-4 text-cyan-400" />
                    <span>DirectX 11 Rendering Pipeline</span>
                  </h3>
                  <span className="text-[10px] px-2 py-0.5 bg-emerald-950 text-emerald-400 border border-emerald-800 font-bold">
                    VERIFIED
                  </span>
                </div>
                <p className="text-xs text-slate-400 mb-3">
                  Located in <code className="text-cyan-300">src/Renderer/D3D11Renderer.cpp</code>
                </p>
                <ul className="text-xs text-slate-300 space-y-1.5 list-disc list-inside">
                  <li>
                    <strong className="text-white">Hardware Feature Level 11_0</strong>: Compatible with AMD Radeon 780M RDNA3 GPU.
                  </li>
                  <li>
                    <strong className="text-white">Double-Buffered SwapChain</strong>: Configured with{' '}
                    <code>DXGI_FORMAT_R8G8B8A8_UNORM</code> and flip-discard.
                  </li>
                  <li>
                    <strong className="text-white">Depth-Stencil Testing</strong>: 24-bit depth + 8-bit stencil buffer (<code>DXGI_FORMAT_D24_UNORM_S8_UINT</code>).
                  </li>
                  <li>
                    <strong className="text-white">Solid & Wireframe States</strong>: Toggleable rasterizer states for engine wireframe inspection.
                  </li>
                </ul>
              </div>

              {/* HLSL Shaders & Lighting */}
              <div className="p-4 bg-slate-900 border border-cyan-500/30">
                <div className="flex items-center justify-between mb-2">
                  <h3 className="text-sm font-bold text-white flex items-center gap-2">
                    <Sparkles className="w-4 h-4 text-cyan-400" />
                    <span>HLSL 5.0 PBR-Lite Shaders</span>
                  </h3>
                  <span className="text-[10px] px-2 py-0.5 bg-emerald-950 text-emerald-400 border border-emerald-800 font-bold">
                    VERIFIED
                  </span>
                </div>
                <p className="text-xs text-slate-400 mb-3">
                  Located in <code className="text-cyan-300">shaders/Basic3D.hlsl</code>
                </p>
                <ul className="text-xs text-slate-300 space-y-1.5 list-disc list-inside">
                  <li>
                    <strong className="text-white">Constant Buffers (b0 & b1)</strong>: 16-byte aligned <code>cbPerFrame</code> (View, Proj, Sun, Fog) and <code>cbPerObject</code> (World, Albedo, Roughness, Emissive).
                  </li>
                  <li>
                    <strong className="text-white">Hemisphere Ambient Light</strong>: Blends between sky blue and dark concrete ground.
                  </li>
                  <li>
                    <strong className="text-white">Blinn-Phong Specular</strong>: Specular highlights calculated from camera view vector and light halfway vector.
                  </li>
                  <li>
                    <strong className="text-white">Atmospheric Fog</strong>: Distance-based exponential fog blending with signature twilight palette.
                  </li>
                </ul>
              </div>

              {/* High-Precision Timer */}
              <div className="p-4 bg-slate-900 border border-cyan-500/30">
                <div className="flex items-center justify-between mb-2">
                  <h3 className="text-sm font-bold text-white flex items-center gap-2">
                    <Gauge className="w-4 h-4 text-cyan-400" />
                    <span>QPC High-Precision Timer</span>
                  </h3>
                  <span className="text-[10px] px-2 py-0.5 bg-emerald-950 text-emerald-400 border border-emerald-800 font-bold">
                    VERIFIED
                  </span>
                </div>
                <p className="text-xs text-slate-400 mb-3">
                  Located in <code className="text-cyan-300">src/Core/Timer.cpp</code>
                </p>
                <ul className="text-xs text-slate-300 space-y-1.5 list-disc list-inside">
                  <li>
                    <strong className="text-white">QueryPerformanceCounter</strong>: 64-bit hardware frequency timer with sub-microsecond resolution.
                  </li>
                  <li>
                    <strong className="text-white">Delta Time Clamping</strong>: Prevents physics explosions and negative values during thread pauses.
                  </li>
                  <li>
                    <strong className="text-white">FPS Telemetry</strong>: Calculates rolling average FPS updated every 250ms on the window title bar.
                  </li>
                </ul>
              </div>
            </div>
          </div>
        </div>

        {/* TAB 4: WINDOWS BUILD & RUN INSTRUCTIONS */}
        <div
          className={`absolute inset-0 overflow-y-auto p-6 bg-slate-950 ${
            activeTab === 'build' ? 'z-10 visible' : 'z-0 invisible pointer-events-none'
          }`}
        >
          <div className="max-w-4xl mx-auto space-y-6">
            <div>
              <h2 className="text-lg font-bold text-white flex items-center gap-2">
                <Terminal className="w-5 h-5 text-cyan-400" />
                <span>BUILD & RUN INSTRUCTIONS (.EXE)</span>
              </h2>
              <p className="text-xs text-slate-400">
                How to compile and run <code className="text-cyan-300">NeonCity.exe</code> on your Windows PC.
              </p>
            </div>

            {/* Quick 1-Click Batch */}
            <div className="p-4 bg-slate-900 border-l-4 border-cyan-400">
              <h3 className="text-sm font-bold text-white mb-1">Method 1: One-Click Build Script (Recommended)</h3>
              <p className="text-xs text-slate-400 mb-3">
                Download the ZIP package, extract it, and double-click <code className="text-cyan-300">build.bat</code>.
              </p>
              <div className="p-3 bg-black/60 border border-slate-800 text-xs text-cyan-300 font-mono">
                neon-city-native\build.bat
              </div>
            </div>

            {/* Manual CMake Commands */}
            <div className="p-4 bg-slate-900 border border-slate-800">
              <h3 className="text-sm font-bold text-white mb-2">Method 2: Command Line (CMake + MSVC)</h3>
              <div className="space-y-2">
                <p className="text-xs text-slate-400">1. Open PowerShell or Command Prompt in the project directory:</p>
                <div className="p-3 bg-black/60 border border-slate-800 text-xs text-emerald-300 font-mono select-all">
                  cd neon-city-native
                </div>

                <p className="text-xs text-slate-400">2. Configure CMake for Visual Studio 2022 (x64):</p>
                <div className="p-3 bg-black/60 border border-slate-800 text-xs text-emerald-300 font-mono select-all">
                  cmake -B build -G "Visual Studio 17 2022" -A x64
                </div>

                <p className="text-xs text-slate-400">3. Compile the Release executable:</p>
                <div className="p-3 bg-black/60 border border-slate-800 text-xs text-emerald-300 font-mono select-all">
                  cmake --build build --config Release
                </div>

                <p className="text-xs text-slate-400">4. Run the native game:</p>
                <div className="p-3 bg-black/60 border border-slate-800 text-xs text-cyan-300 font-mono select-all">
                  .\build\Release\NeonCity.exe
                </div>
              </div>
            </div>

            {/* Visual Studio 2022 IDE Instructions */}
            <div className="p-4 bg-slate-900 border border-slate-800">
              <h3 className="text-sm font-bold text-white mb-2">Method 3: Visual Studio 2022 (IDE Workflow)</h3>
              <ol className="text-xs text-slate-300 space-y-2 list-decimal list-inside">
                <li>Open Visual Studio 2022.</li>
                <li>
                  Click <strong className="text-white">Open a local folder</strong> and select the{' '}
                  <code className="text-cyan-300">neon-city-native</code> folder.
                </li>
                <li>Visual Studio will automatically recognize <code className="text-cyan-300">CMakeLists.txt</code>.</li>
                <li>In the configuration drop-down at the top, select <strong className="text-white">x64-Release</strong>.</li>
                <li>Press <kbd className="px-1.5 py-0.5 bg-slate-800 border border-slate-600 text-white">F5</kbd> or click <strong className="text-white">Build &gt; Build All</strong>.</li>
              </ol>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
