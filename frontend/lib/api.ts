import axios from 'axios';

const API_URL = process.env.NEXT_PUBLIC_API_URL || 'http://localhost:5000';

const api = axios.create({
  baseURL: API_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

export interface Device {
  id: number;
  name: string;
  port: string;
  description?: string;
  createdAt: string;
  readings?: Reading[];
}

export interface Reading {
  id: number;
  deviceId: number;
  energyWh: number;
  durationMin: number;
  createdAt: string;
}

export interface DeviceSummary {
  deviceId: string;
  totalWh: number;
  totalKWh: number;
  custoEstimado: string;
}

export interface ConsumoData {
  periodo: string;
  resumo: {
    totalWh: string;
    totalKWh: string;
    custoTotal: string;
  };
  dispositivos: Array<{
    dispositivo: string;
    porta: string;
    totalWh: string;
    totalKWh: string;
    custoEstimado: string;
    quantidadeLeituras: number;
    tempoTotalMinutos: number;
    ultimaLeitura: string | null;
  }>;
}

// Device API
export const deviceApi = {
  getAll: async (): Promise<Device[]> => {
    const { data } = await api.get('/devices');
    return data;
  },

  getById: async (id: number): Promise<Device> => {
    const { data } = await api.get(`/devices/${id}`);
    return data;
  },

  create: async (device: { name: string; port: string; description?: string }): Promise<Device> => {
    const { data } = await api.post('/devices', device);
    return data;
  },

  update: async (id: number, device: { name?: string; description?: string; port?: string }): Promise<Device> => {
    const { data } = await api.put(`/devices/${id}`, device);
    return data;
  },

  getReadings: async (id: number): Promise<Reading[]> => {
    const { data } = await api.get(`/devices/${id}/readings`);
    return data;
  },

  getSummary: async (id: number): Promise<DeviceSummary> => {
    const { data } = await api.get(`/devices/${id}/summary`);
    return data;
  },
};

// Consumo API
export const consumoApi = {
  getConsumo: async (deviceId?: number, periodo: string = 'total'): Promise<ConsumoData> => {
    const params = new URLSearchParams();
    if (deviceId) params.append('deviceId', deviceId.toString());
    params.append('periodo', periodo);
    
    const { data } = await api.get(`/consumo?${params.toString()}`);
    return data;
  },
};

