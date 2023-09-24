此Repo為對應 15TH鐵人賽所建立，文章內容可參考
https://ithelp.ithome.com.tw/users/20162436/ironman/6345

**Another Little RISC-V ISA Simulator**
​
Another Little RISC-V ISA Simulator(ALISS)是我為這次的30天挑戰的side project所下的標題，顧名思義，
主要的核心目標是要完成一個RISC-V的ISA層級模擬器，以C++進行撰寫，且這個模擬器必須是輕量的，並盡量能達到高效的目標。對標Spike做的100MIPS，我希望在接下來的二十天內所能完成的初版至少要能夠有20MIPS的速度，並會在後續繼續維護及優化。而若要以前面所提到的模擬器種類來進行區分，我們將目標訂為ISA模擬器，也就是"行為模擬器"，因此並不會去模擬時序，但希望能保留將來能擴充時間模擬的彈性。
​
**應用場景**
​
目標是以完成的ALISS進行Linux的開機，目前想要實現的指令集為64bit的I、M、A等三個幾乎是最基本的指令集，並且不會支援MMU的功能。
​
架設Linux會使用Buildroot這套系統協助我們進行架設，使用內建的qemu64-nommu-defconfig並會參考[mini-rv32ima](https://github.com/cnlohr/mini-rv32ima)的patch以及相關設定。
​
**功能**
​
對於ALISS本身，要具備以下功能
​
1. Option Handler : 在command line下指令控制模擬器，包含elf名稱，是否倒log等模式。
2. ELF Loader : 要執行指令可以將ELF檔案 (Executable and Linkable Format)檔案轉換成Binary檔案後放到記憶體進行存取，這一步我希望能夠加在模擬器內部以方便使用者操作。
2. Log 系統 : 希望具備基礎的Log系統，讓開發者可以快速的定位問題點，包含PC-Mode以及Exec Mode。
3. Checkpoint : 由於Linux開機可能需要上千萬到上億道指令，有Checkpoint會更快的幫助我們回到問題點。
​
**開發環境**
​
對於開發環境，要具備以下功能
​
1. 基本的CI/CD/CT環境，這個部分會透過Jenkins建立，以[RISCV-Test](https://github.com/riscv-software-src/riscv-tests) 進行整合測試
2. 基本的單元測試及覆蓋率分析，選用gtest + gcovr進行
​
**開發里程**
​
1. 建立開發環境
2. 完成Option Handler及Elf Loader，並將riscv-test的regression整入
3. 逐步實現I, M, A等指令，每實現一種指令則要通過相應的riscv-test
4. 完成所有指令後開始建立BuildRoots環境並實現Checkpoint功能
5. Linux開機，並以QEMU作為Golden trace的對象
 
**額外目標**
​
1. 達到20MIPS
2. 保留進行Cycle模擬的彈性 (將記憶體的存取切開，將來可能可以改用systemC)
3. CoreMark跑分 (baremetal & on linux)
