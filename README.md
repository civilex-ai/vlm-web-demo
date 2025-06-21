# llama.cpp ile Visual Language Modellerini Kullanma
Bu kılavuz, Büyük Dil Modelleri (LLM’ler) için geliştirilmiş, güçlü ve verimli bir kütüphane olan llama.cpp kullanılarak SmolVLM-500M-Instruct modelinin nasıl çalıştırılacağını açıklamaktadır.

## llama.cpp Nedir ?

llama.cpp, Facebook'un LLaMA modelinin sadelik ve performans odaklı bir C/C++ uyarlamasıdır. Geniş bir donanım yelpazesinde (CPU’lar ve GPU’lar dahil) birçok LLM’i verimli şekilde çalıştırmanıza olanak tanır ve genellikle tüketici sınıfı cihazlarda bile etkileyici bir hız sunar. "quantization" sayesinde, model ağırlıklarını düşük hassasiyette temsil ederek, büyük modellerin bellek tüketimini azaltır ve çıkarım süresini kısaltır. Bu da onu, LLM’leri yerel geliştirme ve deneylerde kullanmak için mükemmel bir tercih haline getirir.

Neden llama.cpp? SmolVLM-500M-Instruct modelini çalıştırmak için llama.cpp’i tercih ediyoruz çünkü:

- Verimlilik: llama.cpp, hızlı çıkarım için son derece optimize edilmiştir, bu sayede pahalı veya özel donanım gerektirmeden modeli yerel cihazda çalıştırabilirsiniz.

- Erişilebilirlik: LLM’lerin çalıştırılması sürecini basitleştirir, deney ve geliştirme için çok daha geniş bir kitleye erişilebilir hale getirir.

- Esneklik: Farklı kuantizasyon seviyelerini destekler ve GPU desteği (varsa) sayesinde performansı daha da artırabilir.

## Gereksinimler

llama.cpp kullanarak SmolVLM-500M-Instruct modelini çalıştırmak için gereksinimler:

- llama.cpp kurulmuş olmalı: llama.cpp’in derlenmiş ve kullanıma hazır olduğundan emin olun. Bu rehber, llama.cpp’in kurulum sürecini kapsamamaktadır.

Not: Modeli manuel olarak indirip kurmanıza gerek yoktur. llama.cpp, sunucuyu ilk kez çalıştırdığınızda modeli otomatik olarak kuracaktır.

### Kurulum Adımları

- llama-server'ı başlatın: 

    Terminalde llama.cpp klasörüne (veya llama-server yürütülebilir dosyasının bulunduğu yere) gidin ve şu komutu çalıştırın:

    ```bash
    ./llama-server -hf ggml-org/SmolVLM-500M-Instruct-GGUF
    ```

    Eğer bir NVIDIA, AMD veya Intel GPU’ya sahipseniz, GPU hızlandırmayı aktif hale getirmek için -ngl 99 bayrağını ekleyebilirsiniz. Bu, modelin katmanlarının büyük bir kısmını GPU’ya yönlendirerek performansı iyileştirir.

    ```bash
    ./llama-server -hf ggml-org/SmolVLM-500M-Instruct-GGUF -ngl 99
    ```

    Windows kullanıcıları için:
    ```bash
    llama-server.exe -hf ggml-org/SmolVLM-500M-Instruct-GGUF -ngl 99
    ```

    SmolVLM dışında farklı modellerle de çalışabilirsiniz. ggml-org/SmolVLM-500M-Instruct-GGUF ifadesini istediğiniz başka bir GGUF modelinin yolu ile değiştirerek farklı modelleri de deneyebilirsiniz. ![Diğer modellere buradan ulaşabilirsiniz.](https://github.com/ggml-org/llama.cpp/blob/master/docs/multimodal.md)

- Web Arayüzünü Açın:

    llama-server çalıştıktan sonra, index.html dosyasını açın.

* "Başlat"a tıklayın ve VLM modelleriyle etkileşimin tadını çıkarın!

