# NoahGameFrame Wiki - Klasör Yapısı ve Kullanım Kılavuzu

## 📋 İçindekiler
- [Genel Bakış](#genel-bakış)
- [Klasör Yapısı](#klasör-yapısı)
- [Sunucu Türleri](#sunucu-türleri)
- [Plugin Sistemi](#plugin-sistemi)
- [Konfigürasyon](#konfigürasyon)
- [Derleme ve Çalıştırma](#derleme-ve-çalıştırma)
- [Geliştirme Rehberi](#geliştirme-rehberi)
- [Troubleshooting](#troubleshooting)

---

## 🎯 Genel Bakış

NoahGameFrame, C++ ile yazılmış modüler, ölçeklenebilir ve dağıtık bir oyun sunucusu framework'üdür. Plugin tabanlı mimarisi sayesinde kolayca genişletilebilir ve özelleştirilebilir.

### ✨ Temel Özellikler
- **Plugin Tabanlı Mimari** - Modüler geliştirme
- **Lua Script Desteği** - Hot reload özelliği
- **WebSocket/HTTP Desteği** - Modern web teknolojileri
- **Çoklu Sunucu Desteği** - Dağıtık mimari
- **Cross-Platform** - Windows ve Linux desteği

---

## 📁 Klasör Yapısı

### 🎯 Ana Klasörler

#### `_Out/` - Çıktı ve Çalıştırma Klasörü
```
_Out/
├── Debug/              # Debug modunda derlenmiş sunucular
├── Release/            # Release modunda derlenmiş sunucular
├── NFDataCfg/         # Konfigürasyon dosyaları
│   ├── Debug/         # Debug konfigürasyonları
│   ├── Release/       # Release konfigürasyonları
│   ├── Ini/           # Sunucu ayarları
│   ├── ScriptModule/  # Lua script'leri
│   ├── Excel/         # Veri dosyaları
│   └── Struct/        # Veri yapıları
├── logs/              # Sunucu log dosyaları
├── rund.bat          # Debug sunucuları çalıştır
├── runr.bat           # Release sunucuları çalıştır
├── Tutorial.bat       # Tutorial sunucusu çalıştır
└── websocket_chat_demo.htm  # WebSocket test sayfası
```

#### `NFComm/` - Temel Framework Bileşenleri
```
NFComm/
├── NFCore/            # Çekirdek sistem
├── NFKernelPlugin/    # Kernel plugin'i
├── NFNetPlugin/       # Ağ iletişimi
├── NFLuaScriptPlugin/ # Lua script desteği
├── NFLogPlugin/       # Loglama sistemi
├── NFNavigationPlugin/ # Yol bulma sistemi
├── NFConfigPlugin/    # Konfigürasyon yönetimi
├── NFActorPlugin/     # Actor model
├── NFNoSqlPlugin/     # NoSQL veritabanı desteği
└── NFSecurityPlugin/  # Güvenlik sistemi
```

#### `NFServer/` - Sunucu Plugin'leri
```
NFServer/
├── NFMasterServerPlugin/     # Ana sunucu
├── NFGameServerPlugin/       # Oyun sunucusu
├── NFLoginServerPlugin/      # Giriş sunucusu
├── NFWorldServerPlugin/      # Dünya sunucusu
├── NFDBServerPlugin/         # Veritabanı sunucusu
├── NFProxyServerPlugin/      # Proxy sunucusu
└── NF*Net_*Plugin/          # Ağ iletişim plugin'leri
```

#### `NFExamples/` - Örnek Plugin'ler
```
NFExamples/
├── NFChatPlugin/             # Chat sistemi
├── NFInventoryPlugin/        # Envanter sistemi
├── NFConsumeManagerPlugin/   # Tüketim yöneticisi
└── NFServer/                # Ana sunucu örneği
```

#### `Tutorial/` - Öğretici Örnekler
```
Tutorial/
├── Tutorial1/               # Hello World
├── Tutorial2/               # Data Driver
├── Tutorial3/               # Event System
├── Tutorial4/               # Actor Model
├── Tutorial5/               # Advanced Features
├── Tutorial6/               # Network
└── Tutorial7/               # Complete Example
```

#### `Dependencies/` - Harici Kütüphaneler
```
Dependencies/
├── hiredis/                 # Redis client
├── lua/                     # Lua scripting
├── concurrentqueue/          # Thread-safe queue
├── navigation/              # Yol bulma algoritması
├── protobuf/                # Google Protocol Buffers
├── RapidXML/                # XML parser
└── nlohmann/                # JSON library
```

---

## 🖥️ Sunucu Türleri

### Sunucu Mimarisi
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   ProxyServer   │    │   LoginServer   │    │  MasterServer   │
│   (Port: 25001) │    │   (Port: 14001) │    │   (Port: 13001) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
         ┌───────────────────────┼───────────────────────┐
         │                       │                       │
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  GameServer     │    │  WorldServer    │    │   DBServer      │
│  (Port: 16001)  │    │  (Port: 17001)  │    │   (Port: 18001) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### Sunucu Rolleri

| Sunucu | ID | Port | Rol | Bağlantı Türü |
|--------|----|----- |-----|---------------|
| **MasterServer** | 3 | 13001 | Sunucu yönetimi ve koordinasyon | Internal + External |
| **LoginServer** | 4 | 14001 | Kullanıcı girişi ve kimlik doğrulama | External |
| **ProxyServer** | 5 | 25001 | WebSocket/HTTP proxy | External |
| **GameServer** | 6+ | 16001+ | Oyun mantığı ve işlemleri | External |
| **WorldServer** | 7 | 17001 | Dünya durumu ve senkronizasyon | Internal Only |
| **DBServer** | 8 | 18001 | Veritabanı işlemleri | Internal Only |

### 🔍 Sunucu Durumu Kontrolü

#### Port Dinleme Durumu
```cmd
# Tüm sunucu portlarını kontrol et
netstat -an | findstr "13001\|14001\|15001\|16001\|17001\|18001\|25001"

# Çalışan NFServer process'lerini kontrol et
tasklist /fi "imagename eq NFServer.exe" /v
```

#### Beklenen Durum
- ✅ **External Ports** (Client bağlantıları): 13001, 14001, 16001, 25001
- ❌ **Internal Ports** (Sunucu arası): 17001, 18001 (normalde dinlemez)
- ✅ **Redis Port**: 6379 (aktif bağlantılar olmalı)

#### Sunucu Başlatma Sırası
1. **Master Server** (ID=3) - Koordinasyon merkezi
2. **World Server** (ID=7) - Internal communication
3. **Login Server** (ID=4) - Client authentication
4. **DB Server** (ID=8) - Database operations
5. **Game Server** (ID=16001) - Game logic
6. **Proxy Server** (ID=5) - WebSocket/HTTP gateway

---

## 🔌 Plugin Sistemi

### Plugin Yaşam Döngüsü
```cpp
class NFIModule
{
public:
    virtual bool Awake();      // Plugin başlatma
    virtual bool Init();       // Plugin başlatma
    virtual bool AfterInit();  // Başlatma sonrası
    virtual bool Execute();    // Ana döngü
    virtual bool BeforeShut(); // Kapatma öncesi
    virtual bool Shut();       // Plugin kapatma
};
```

### Plugin Türleri

#### **Temel Plugin'ler**
- **NFKernelPlugin** - Çekirdek sistem
- **NFNetPlugin** - Ağ iletişimi
- **NFLogPlugin** - Loglama
- **NFConfigPlugin** - Konfigürasyon

#### **Sunucu Plugin'leri**
- **NFMasterServerPlugin** - Ana sunucu
- **NFGameServerPlugin** - Oyun sunucusu
- **NFLoginServerPlugin** - Giriş sunucusu

#### **Örnek Plugin'ler**
- **NFChatPlugin** - Chat sistemi
- **NFInventoryPlugin** - Envanter sistemi

---

## ⚙️ Konfigürasyon

### Ana Konfigürasyon Dosyaları

#### `Server.xml` - Sunucu Ayarları
```xml
<Object Id="GameServer_6" 
        Area="1" 
        CpuCount="1" 
        IP="127.0.0.1" 
        MaxOnline="5000" 
        Port="16006" 
        ServerID="6" 
        Type="6" />
```

#### `Plugin.xml` - Plugin Konfigürasyonu
```xml
<GameServer>
    <Plugin Name="NFNetPlugin" />
    <Plugin Name="NFKernelPlugin" />
    <Plugin Name="NFGameServerPlugin" />
</GameServer>
```

#### `Tutorial.xml` - Tutorial Konfigürasyonu
```xml
<GameServer>
    <Plugin Name="Tutorial1" />
    <Plugin Name="Tutorial2" />
    <Plugin Name="Tutorial5" />
</GameServer>
```

---

## 🔨 Derleme ve Çalıştırma

### Visual Studio ile Derleme

#### Debug Modunda Derleme
1. Visual Studio'yu açın
2. `NoahFrame.sln` dosyasını açın
3. Configuration: **Debug**
4. Platform: **x64**
5. **Build → Build Solution** (F7)

#### Release Modunda Derleme
1. Configuration: **Release**
2. Platform: **x64**
3. **Build → Build Solution** (F7)

### Sunucuları Çalıştırma

#### Debug Modunda
```cmd
cd _Out
rund.bat
```

#### Release Modunda
```cmd
cd _Out
runr.bat
```

#### Tutorial Sunucusu
```cmd
cd _Out
Tutorial.bat
```

### WebSocket Test
```html
<!-- websocket_chat_demo.htm -->
Server: ws://127.0.0.1:25001
```

---

## 💻 Geliştirme Rehberi

### Yeni Plugin Oluşturma

#### 1. Plugin Klasörü Oluştur
```
NFExamples/
└── NFMyPlugin/
    ├── NFMyPlugin.h
    ├── NFMyPlugin.cpp
    └── NFMyPlugin.txt
```

#### 2. Plugin Header Dosyası
```cpp
// NFMyPlugin.h
#ifndef NF_MY_PLUGIN_H
#define NF_MY_PLUGIN_H

#include "NFComm/NFPluginModule/NFIPluginManager.h"

class NFMyPlugin : public NFIPlugin
{
public:
    NFMyPlugin(NFIPluginManager* p) : NFIPlugin(p) {}
    virtual const int GetPluginVersion();
    virtual const std::string GetPluginName();
    virtual void Install();
    virtual void Uninstall();
};

#endif
```

#### 3. Plugin Implementation
```cpp
// NFMyPlugin.cpp
#include "NFMyPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

const int NFMyPlugin::GetPluginVersion()
{
    return 0;
}

const std::string NFMyPlugin::GetPluginName()
{
    return "NFMyPlugin";
}

void NFMyPlugin::Install()
{
    REGISTER_MODULE(pPluginManager, NFIMyModule, NFMyModule)
}

void NFMyPlugin::Uninstall()
{
    UNREGISTER_MODULE(pPluginManager, NFIMyModule, NFMyModule)
}
```

### Lua Script Geliştirme

#### Script Dosyası Oluştur
```lua
-- ScriptModule/my_module.lua
my_module = {}
register_module(my_module, "my_module");

function my_module.awake()
    print("My module awake!")
end

function my_module.init()
    print("My module init!")
end

function my_module.after_init()
    print("My module after_init!")
end

function my_module.execute()
    -- Ana döngü
end

function my_module.before_shut()
    print("My module before_shut!")
end

function my_module.shut()
    print("My module shut!")
end
```

#### Script Listesine Ekle
```lua
-- ScriptModule/script_list.lua
ScriptList={
    {tbl=nil, tblName="MyModule"},
}
load_script_file(ScriptList)
```

---

## 🐛 Troubleshooting

### Yaygın Sorunlar

#### 1. Runtime Library Uyumsuzluğu
```
LNK2038: 'RuntimeLibrary' için uyuşmazlık algılandı
```
**Çözüm:**
- Clean Solution yapın
- Rebuild Solution yapın
- Tüm kütüphaneleri aynı modda derleyin

#### 2. Sunucu Bulunamadı Hatası
```
Cannot find current server, AppID = 6
```
**Çözüm:**
- `Server.xml` dosyasında sunucu ID'sini kontrol edin
- GameServer ID=6 için konfigürasyon ekleyin

#### 3. WebSocket Bağlantı Hatası
```
WebSocket error: 800030
```
**Çözüm:**
- Port 25001'in açık olduğunu kontrol edin
- ProxyServer'ın çalıştığını kontrol edin

#### 5. Sunucu Port Dinleme Sorunu
```
World Server (17001) ve DB Server (18001) portları dinlemiyor
```
**Çözüm:**
- Bu **normal bir durumdur**! Bu sunucular internal communication için tasarlanmıştır
- Client'lar sadece Proxy (25001), Login (14001) ve Game Server (16001) portlarına bağlanır
- World ve DB Server'lar diğer sunucularla internal olarak iletişim kurar

#### 6. Sunucu Başlatma Sorunu
```
Bazı sunucular çalışmıyor veya port dinlemiyor
```
**Çözüm:**
- `rund.bat` dosyasını çalıştırın
- Sunucuların başlatma sırasını bekleyin (choice komutları ile gecikme var)
- Redis'in çalıştığını kontrol edin: `netstat -an | findstr "6379"`
- Process'leri kontrol edin: `tasklist /fi "imagename eq NFServer.exe"`

#### 7. Sunucu Bağlantı Sorunu
```
Client sunucuya bağlanamıyor
```
**Çözüm:**
- External portları kontrol edin: 13001, 14001, 16001, 25001
- Firewall ayarlarını kontrol edin
- IP adresini kontrol edin (127.0.0.1 veya gerçek IP)
- Sunucu loglarını kontrol edin

### Log Dosyaları

#### Log Konumları
```
_Out/logs/
├── master_server_debug_YYYYMMDD.log
├── login_server_debug_YYYYMMDD.log
├── game_server_debug_YYYYMMDD.log
├── world_server_debug_YYYYMMDD.log
├── db_server_debug_YYYYMMDD.log
└── proxy_server_debug_YYYYMMDD.log
```

#### Log Seviyeleri
- **INFO** - Bilgi mesajları
- **WARNING** - Uyarı mesajları
- **ERROR** - Hata mesajları
- **DEBUG** - Debug mesajları

---

## 📚 Ek Kaynaklar

### Önemli Dosyalar
- **`README.md`** - Proje açıklaması
- **`LICENSE`** - Lisans bilgisi
- **`CMakeLists.txt`** - CMake konfigürasyonu
- **`NoahFrame.sln`** - Visual Studio solution

### Build Scriptleri
- **`buildServer.sh`** - Linux build script
- **`build_dep.bat`** - Windows dependency build
- **`install4cmake.sh`** - Linux installation

### Docker Desteği
- **`docker/Dockerfile`** - Docker image
- **`docker/run.sh`** - Docker çalıştırma

---

## 🎯 Hızlı Başlangıç

### 1. Projeyi Derle
```cmd
# Visual Studio ile
# Build → Build Solution (F7)
```

### 2. Sunucuları Çalıştır
```cmd
cd _Out
rund.bat  # Debug modunda
# veya
runr.bat  # Release modunda
```

### 3. Sunucu Durumunu Kontrol Et
```cmd
# Port durumunu kontrol et
netstat -an | findstr "13001\|14001\|16001\|25001"

# Process durumunu kontrol et
tasklist /fi "imagename eq NFServer.exe"

# Redis bağlantısını kontrol et
netstat -an | findstr "6379"
```

### 4. WebSocket Test Et
```html
# websocket_chat_demo.htm dosyasını aç
# ws://127.0.0.1:25001 adresine bağlan
```

### 5. Tutorial Öğren
```cmd
cd _Out
Tutorial.bat
```

---

## 📞 Destek

### GitHub
- **Repository:** https://github.com/ketoo/NoahGameFrame
- **Wiki:** https://github.com/ketoo/NoahGameFrame/wiki
- **Issues:** https://github.com/ketoo/NoahGameFrame/issues

### Topluluk
- **QQ Group:** 330241037
- **Gitter Chat:** https://gitter.im/ketoo/NoahGameFrame

---

*Bu wiki dosyası NoahGameFrame v1.0 için hazırlanmıştır. Güncellemeler için GitHub repository'sini takip edin.*
