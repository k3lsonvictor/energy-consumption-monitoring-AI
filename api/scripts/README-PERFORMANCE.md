# 🚀 Testes de Desempenho da API

Este diretório contém scripts para testar o desempenho da API e coletar métricas importantes.

## 📊 Métricas Coletadas

- **Tempo médio de resposta**: Latência média das requisições
- **Máximo de requisições/segundo**: Throughput máximo da API
- **Taxa de erros (HTTP 4xx/5xx)**: Percentual de requisições com erro
- **Disponibilidade estimada**: Taxa de sucesso das requisições

## 🛠️ Scripts Disponíveis

### 1. Script Customizado (Node.js puro)

**Arquivo**: `performance-test.js`

**Uso básico**:
```bash
npm run test:perf
```

**Com variáveis de ambiente**:
```bash
# Teste rápido (10s, 100 requisições, 5 concorrentes)
npm run test:perf:quick

# Teste de stress (60s, 5000 requisições, 50 concorrentes)
npm run test:perf:stress

# Configuração customizada
API_URL=http://localhost:5000 \
TOTAL=1000 \
CONCURRENT=20 \
DURATION=30 \
node scripts/performance-test.js
```

**Variáveis de ambiente**:
- `API_URL`: URL da API (padrão: `http://localhost:5000`)
- `TOTAL`: Total de requisições (padrão: `1000`)
- `CONCURRENT`: Requisições concorrentes (padrão: `10`)
- `DURATION`: Duração do teste em segundos (padrão: `30`)

### 2. Script com Autocannon

**Arquivo**: `performance-test-autocannon.js`

**Instalação**:
```bash
npm install -g autocannon
```

**Uso**:
```bash
npm run test:perf:autocannon
```

**Com variáveis de ambiente**:
```bash
API_URL=http://localhost:5000 \
DURATION=30 \
CONNECTIONS=10 \
PIPELINING=1 \
node scripts/performance-test-autocannon.js
```

## 📈 Interpretando os Resultados

### Tempo de Resposta

- **Média**: Tempo médio de resposta (ideal: < 200ms)
- **P50 (Mediana)**: 50% das requisições respondem em menos tempo
- **P95**: 95% das requisições respondem em menos tempo (ideal: < 500ms)
- **P99**: 99% das requisições respondem em menos tempo (ideal: < 1000ms)

### Throughput

- **Requisições/segundo**: Quantas requisições a API consegue processar por segundo
- Quanto maior, melhor a capacidade de processamento

### Taxa de Erros

- **4xx (Erros de Cliente)**: Geralmente problemas de validação ou autenticação
- **5xx (Erros de Servidor)**: Problemas internos da API (ideal: 0%)

### Disponibilidade

- **99.9%+**: 🟢 Excelente
- **99.0%+**: 🟡 Bom
- **95.0%+**: 🟠 Aceitável
- **< 95.0%**: 🔴 Crítico

## 📝 Relatórios

Os scripts geram relatórios em JSON:

- `performance-report.json` (script customizado)
- `performance-report-autocannon.json` (script autocannon)

## 🔍 Exemplo de Saída

```
📈 RELATÓRIO DE DESEMPENHO
============================================================

⏱️  TEMPO DE RESPOSTA:
   Média:        45.23 ms
   Mínimo:       12.34 ms
   Máximo:       234.56 ms
   P50 (mediana): 42.10 ms
   P95:          89.45 ms
   P99:          156.78 ms

🚀 THROUGHPUT:
   Requisições/segundo: 125.50 req/s
   Duração total:      30.00 segundos

📊 REQUISIÇÕES:
   Total:        1000
   Sucesso:      995 (99.50%)
   Falhas:       5 (0.50%)

❌ ERROS HTTP:
   4xx (Cliente): 3
   5xx (Servidor): 2
   Taxa de erro:  0.50%

✅ DISPONIBILIDADE:
   Taxa de sucesso: 99.50%
   Status: 🟢 EXCELENTE (99.9%+)
```

## 🎯 Dicas

1. **Execute testes em ambiente de desenvolvimento primeiro** para validar a configuração
2. **Aumente gradualmente a carga** para encontrar os limites da API
3. **Monitore recursos do servidor** (CPU, memória, banco de dados) durante os testes
4. **Compare resultados** antes e depois de otimizações
5. **Execute testes regulares** para detectar regressões de desempenho

## 🔧 Troubleshooting

### Erro: "ECONNREFUSED"
- Verifique se a API está rodando
- Confirme a URL correta em `API_URL`

### Teste muito lento
- Reduza `TOTAL` ou `CONCURRENT`
- Verifique se há problemas de rede ou banco de dados

### Muitos erros 5xx
- Verifique logs da API
- Monitore recursos do servidor
- Pode indicar sobrecarga ou problemas no banco de dados

