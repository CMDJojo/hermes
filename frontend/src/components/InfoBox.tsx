import { motion } from 'framer-motion';
import React from 'react';

import '../styles/InfoBox.css';

interface InfoBoxProps {
  title: string;
  children: React.ReactNode;
}

export default function InfoBox({ title, children }: InfoBoxProps) {
  return (
    <motion.div className="InfoBox">
      <h2>{title}</h2>
      {children}
    </motion.div>
  );
}
