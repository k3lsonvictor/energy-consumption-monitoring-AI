# Frontend - Gerenciador de Dispositivos IoT

Frontend em Next.js para gerenciamento e visualização de dispositivos IoT e monitoramento de consumo de energia.

## Funcionalidades

- 📊 **Dashboard**: Visão geral de todos os dispositivos com estatísticas de consumo
- 🔌 **Dispositivos**: Lista completa de dispositivos com busca e filtros
- 📈 **Detalhes**: Página detalhada de cada dispositivo com gráficos de consumo
- ⚙️ **Gerenciamento**: CRUD completo para dispositivos (criar, editar, visualizar)

## Tecnologias

- **Next.js 14** - Framework React
- **TypeScript** - Tipagem estática
- **Tailwind CSS** - Estilização
- **Recharts** - Gráficos e visualizações
- **Axios** - Cliente HTTP
- **Lucide React** - Ícones

## Instalação

1. Instale as dependências:

```bash
npm install
```

2. Configure a variável de ambiente:

Crie um arquivo `.env.local` na raiz do projeto:

```env
NEXT_PUBLIC_API_URL=http://localhost:3000
```

Ajuste a URL conforme necessário (se sua API estiver rodando em outra porta).

3. Execute o servidor de desenvolvimento:

```bash
npm run dev
```

O frontend estará disponível em [http://localhost:3001](http://localhost:3001)

## Estrutura do Projeto

```
frontend/
├── app/                    # Páginas e rotas (App Router)
│   ├── dashboard/         # Página principal
│   ├── devices/           # Páginas de dispositivos
│   │   ├── [id]/         # Detalhes do dispositivo
│   │   └── manage/       # Gerenciamento
│   └── layout.tsx        # Layout principal
├── components/            # Componentes reutilizáveis
│   ├── Header.tsx        # Cabeçalho com navegação
│   ├── DeviceCard.tsx    # Card de dispositivo
│   └── StatsCard.tsx     # Card de estatísticas
└── lib/                  # Utilitários e serviços
    └── api.ts            # Cliente API
```

## Rotas

- `/` - Redireciona para `/dashboard`
- `/dashboard` - Dashboard principal com visão geral
- `/devices` - Lista de todos os dispositivos
- `/devices/[id]` - Detalhes de um dispositivo específico
- `/devices/manage` - Gerenciamento de dispositivos (CRUD)

## API

O frontend se comunica com a API backend através dos seguintes endpoints:

- `GET /devices` - Lista todos os dispositivos
- `GET /devices/:id` - Busca dispositivo por ID
- `POST /devices` - Cria novo dispositivo
- `PUT /devices/:id` - Atualiza dispositivo
- `GET /devices/:id/readings` - Lista leituras do dispositivo
- `GET /devices/:id/summary` - Resumo do dispositivo
- `GET /consumo` - Dados de consumo (com filtros de período)

## Build para Produção

```bash
npm run build
npm start
```

## Desenvolvimento

O projeto usa:
- **TypeScript** para type safety
- **Tailwind CSS** para estilização
- **ESLint** para linting
- **App Router** do Next.js 14

