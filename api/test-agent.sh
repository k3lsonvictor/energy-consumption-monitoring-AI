#!/bin/bash

# Script para testar o agente de IA
# Uso: ./test-agent.sh "Qual o consumo hoje?"

API_URL="http://localhost:5000/test/chat"
MENSAGEM="${1:-Qual o consumo hoje?}"

echo "🧪 Testando agente de IA..."
echo "📨 Mensagem: $MENSAGEM"
echo ""

curl -X POST "$API_URL" \
  -H "Content-Type: application/json" \
  -d "{\"mensagem\": \"$MENSAGEM\"}" \
  | jq '.'

echo ""
echo "✅ Teste concluído!"

