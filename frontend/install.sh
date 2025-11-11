#!/bin/bash

echo "🚀 Instalando dependências do frontend..."
cd "$(dirname "$0")"
npm install

echo "✅ Instalação concluída!"
echo ""
echo "📝 Próximos passos:"
echo "1. Crie um arquivo .env.local com:"
echo "   NEXT_PUBLIC_API_URL=http://localhost:3000"
echo ""
echo "2. Execute o servidor de desenvolvimento:"
echo "   npm run dev"
echo ""

