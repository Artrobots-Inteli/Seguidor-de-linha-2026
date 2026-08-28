# Documentação dos encontros de desenvolvimento:

### 03/06
#### Decisões de organização

Todos irão transitar entre áreas, primeiramente, todos focaram seus esforços no entendimento de mecânica, para construir e agilizar a construção. Conforme a demanda de mecânica e elétrica diminuem, o grupo começará a parte de computação.
O product Owner (Vitor) será um facilitador para o grupo, ajudando em tarefas e se compretendo a trazer "auto estudos" para todo o grupo, seguindo as demandas.
O grupo se compromete a estudar todos os autos estudos e, caso contrário, a documentação semanal será responsável por demonstrar todas as ações realizadas e os "auto estudos" feitos.

#### Decisões e próximos passos:
O grupo optou por começar pela estrutura física do seguidor, focando principalmente na estrutura e, consequentemente, pela precisão e a leveza dos componenetes. Como terceira etapa, teste de funcionalidade focando na velocidade do robo.

Seguem os componentes pesquisados e de possíveis implementações:

### Documentação semanal 

#### Progresso semanal de cada integrantes 

**João Pedro**

| Motor | RPM Sem Carga | Potência Nominal (V) | Peso (g) | Torque Nominal (mNm) | Preço (R$) |
|-------|---------------|----------------------|----------|----------------------|------------|
| Micro Motor com Caixa de Redução (atual) | 750 | 6 | 20 | 166,71 | 68 |
| Maxon DCX10L 4.5V (Raiju) | 12000 | 4.5 | 11 | 2,12 | 603,7 |
| RIC-1020DT-037450 (sugestão) | 39500 | 7.4 | 9 | 1,55 | 45-75 |
| 1020 coreless motor 7.4v neodymium (sugestão) | 22000 | 7.4 | 9 | 2 | 25-40 |

| Peça | Largura (mm) | Comprimento (mm) | Altura (mm) | Material |
|-------|---------------|----------------------|----------|----------------------|
| Sensor em linha | 16.24 | 66.1 | 3,1 (sem pinos) | N/A |
| Parte superior do chifre | 24.82 | 99.65 | 3.1 (chapa única)/ 9.03(chapa tripla) | MDF |
| Parte superior do chifre (vazamento) | 15.02 | 30.85 | 3.1 | MDF |
| Parte inferior do chifre (cabeça) | 26.90 | 69.80 | 2.51/7.69 | MDF |
| Parte inferior do chifre (ligação) | 18.23/6.82 | 66.15/53.53 | 2.54 | MDF |
| Placa do eixo | 128.58 | 101.75 | 1.44 | Não sei |
| Placa extra | 101.44 | 151.06 | 1.61 | Não sei |
| Case do motor | 37.42 | 29.90 | 12.05 | impressão 3D |
| Roda | 16,00 | 22 (diâmetro externo)/ 3 (diâmetro externo) | N/A | Borracha/Metal |
| Módulo Driver TB6612FNG | 18 | 20 | 2.16 | N/A |
| Arduino Nano | 18 | 45 | 3.71 (com porta usb) | N/A |


**Kaylan**

**Matheus**
### **Pesquisa sobre estrutura**
Dentro da pesquisa, foi utilizado como principal referência o Raijin V1, da equipe Raijū. A ideia foi entender quais decisões fazem sentido para o projeto e quais estariam muito fora do escopo.
</br>
**Resumo**
</br>
- **PCB como chassi:** a solução utilizada no Raijin V1 reduz peso e integra estrutura e eletrônica, mas não deve ser uma prioridade atual. Antes disso, é necessário validar o formato físico do robô em um chassi comum e verificar se a equipe possui estrutura e tempo para desenvolver uma PCB estrutural.
- **Cabeçote dos sensores:** a posição dos sensores é essencial para a precisão do robô. Por isso, o uso de um cabeçote ajustável é uma boa solução para testes no primeiro protótipo.
- **Escolha dos sensores:** O modelo dos sensores escolhidos influencia a altura de leitura, a quantidade de sensores, o tamanho do cabeçote e a precisão da detecção da linha.
- **Impressão 3D:** a impressão 3D deve ser usada apenas em partes específicas, como suporte dos sensores, fixações, mancais e peças de encaixe. Contudo, outras alternativas mais leves devem ser testadas no protótipo como chapas finas, acrílico, fenolite, fibra de carbono ou MDF fino para prototipagem.
- **Ventoinhas:** apesar de o Raijin utilizar ventoinhas para aumentar a aderência em curvas, essa solução não deve ser foco inicial do projeto. Elas aumentam a complexidade, o consumo de bateria, o peso e a vibração. Para compensar sua ausência, o grupo deve priorizar rodas com boa aderência, centro de massa baixo e boa distribuição de peso.
- **Uso de quatro rodas:** a configuração com quatro rodas aumenta a estabilidade, tração e distribuição de peso mas traz maior consumo de energia, peso e complexidade mecânica. 

#### Ponto 1 - PCB como chassi
Um dos pontos interessantes do Raijin é que a própria PCB (Placa de Circuito Impressa) funciona como chassi. Isso ajuda a reduzir o peso, porque evita uma estrutura separada apenas para sustentar a eletrônica. Dentro do nosso contexto, essa solução parece bem acertada mas não deve ser um foco do desenvolvimento. Primeiramente, teria que verificar se existe a estrutura para a equipe fazer essa impressão. Além disso, os membros da equipe teriam de dedicar tempo útil para o aprendizado de uma feature que pode acabar mais atrapalhando a construção da estrutura do que ajudando.
Ademais, antes de transformar a PCB em chassi, é preciso validar o formato físico do robô. O ideal é montar primeiro um protótipo de chassi comum, testar proporções, posição dos sensores, bateria, motores e rodas, e só depois estudar uma versão em que a PCB também funcione como estrutura.

#### Ponto 2 - Cabeçote dos sensores
Outro ponto importante é a posição dos sensores. No Raijin, os sensores ficam em uma parte frontal, separada da base principal por uma espécie de “pescoço”. Isso é importante porque o robô precisa detectar a curva antes de chegar nela. A distância do cabeçote dos sensores é uma das partes essenciais do projeto, haja vista que se os sensores ficarem muito perto das rodas, o robô reage tardiamente e se ficarem muito longe, ele pode corrigir antes da hora e ficar instável.
Portanto, uma sugestão é que o primeiro protótipo tenha um cabeçote ajustável. Assim, dá para testar diferentes distâncias até encontrar uma posição boa.
Também é necessário definir quais sensores serão usados antes de fechar o desenho da frente do robô. O modelo dos sensores de cor/IR/luminosidade vai definir a altura de leitura e a quantidade necessária.
#### Ponto 3 - Impressão 3D
No Raijin, a impressão 3D aparece em partes específicas, como o pescoço dos sensores, mancais dos motores e engrenagens. Essa é uma boa lógica: usar impressão 3D onde ela resolve um problema específico. Para o nosso robô, vale pesquisar alternativas mais leves, como chapas finas, acrílico, fibra de carbono, fenolite ou um MDF fino para protótipo.
#### Ponto 4 - Ventoinhas
O Raijin utiliza ventoinhas para aumentar a força normal do robô sem aumentar sua massa. Isso ajuda o robô a ter mais aderência nas curvas. Porém, elas aumentam a complexidade da estrutura, exigem mais bateria (o que adiciona mais peso consequentemente), adicionam vibração e tornam o controle mais difícil. 
Para compensar a ausência das ventoinhas, devemos focar em soluções mais simples:
- Rodas com boa aderência
- Centro de massa baixo
#### Uso de quatro rodas
A configuração de quatro rodas ajuda na distribuição de peso e na potência do robô. Entretanto, seu uso também gera trade-offs como maior consumo de energia e complexidade mecânica. Outrossim, dependendo da montagem, o robô pode encontrar mais dificuldades em realizar curvas de maior angulação.

---

**Pablo**

**Ricardo**

**Vinicius**
